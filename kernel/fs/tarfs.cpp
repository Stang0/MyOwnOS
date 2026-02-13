#include "tarfs.hpp"
#include "../drivers/console.hpp"
#include "../drivers/serial.hpp"

struct TarHeader {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char padding[12];
};

static const uint8_t* g_archive = nullptr;
static size_t g_archive_size = 0;
static bool g_ready = false;

static size_t align_up_512(size_t n) {
    return (n + 511) & ~((size_t)511);
}

static bool is_space(char c) {
    return c == ' ' || c == '\0';
}

static size_t octal_to_size(const char* s, size_t len) {
    size_t value = 0;
    size_t i = 0;
    while (i < len && is_space(s[i])) i++;
    for (; i < len; i++) {
        char c = s[i];
        if (c < '0' || c > '7') break;
        value = (value << 3) + (size_t)(c - '0');
    }
    return value;
}

static bool is_zero_block(const uint8_t* p) {
    for (size_t i = 0; i < 512; i++) {
        if (p[i] != 0) return false;
    }
    return true;
}

static const char* skip_dot_slash(const char* s) {
    while (s[0] == '.' && s[1] == '/') s += 2;
    return s;
}

static size_t str_len(const char* s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

static size_t trimmed_len_no_trailing_slash(const char* s) {
    const char* t = skip_dot_slash(s);
    size_t n = str_len(t);
    while (n > 0 && t[n - 1] == '/') n--;
    return n;
}

static bool path_eq_normalized(const char* a, const char* b) {
    const char* an = skip_dot_slash(a);
    const char* bn = skip_dot_slash(b);
    size_t al = trimmed_len_no_trailing_slash(an);
    size_t bl = trimmed_len_no_trailing_slash(bn);
    if (al != bl) return false;
    for (size_t i = 0; i < al; i++) {
        if (an[i] != bn[i]) return false;
    }
    return true;
}

static void tar_build_path(const TarHeader* hdr, char* out, size_t out_size) {
    size_t p = 0;
    if (hdr->prefix[0] != '\0') {
        for (size_t i = 0; i < sizeof(hdr->prefix) && hdr->prefix[i] != '\0'; i++) {
            if (p + 1 >= out_size) return;
            out[p++] = hdr->prefix[i];
        }
        if (p + 1 < out_size) {
            out[p++] = '/';
        }
    }
    for (size_t i = 0; i < sizeof(hdr->name) && hdr->name[i] != '\0'; i++) {
        if (p + 1 >= out_size) return;
        out[p++] = hdr->name[i];
    }
    out[p] = '\0';
}

static bool tar_walk_next(size_t* offset, const TarHeader** hdr_out, const uint8_t** data_out, size_t* size_out) {
    if (*offset + 512 > g_archive_size) return false;
    const TarHeader* hdr = (const TarHeader*)(g_archive + *offset);

    if (is_zero_block((const uint8_t*)hdr)) {
        return false;
    }

    size_t file_size = octal_to_size(hdr->size, sizeof(hdr->size));
    size_t data_off = *offset + 512;
    size_t next_off = data_off + align_up_512(file_size);
    if (next_off > g_archive_size) return false;

    *hdr_out = hdr;
    *data_out = g_archive + data_off;
    *size_out = file_size;
    *offset = next_off;
    return true;
}

bool tarfs_init(const uint8_t* archive, size_t size) {
    g_archive = archive;
    g_archive_size = size;
    g_ready = false;

    if (!archive || size < 1024) {
        kprint("[TARFS] Archive missing or too small.\n");
        return false;
    }

    const TarHeader* hdr = (const TarHeader*)archive;
    if (!(hdr->magic[0] == 'u' && hdr->magic[1] == 's' && hdr->magic[2] == 't' &&
          hdr->magic[3] == 'a' && hdr->magic[4] == 'r')) {
        kprint("[TARFS] Invalid archive header.\n");
        return false;
    }

    g_ready = true;
    kprint("[TARFS] Initialized.\n");
    return true;
}

bool tarfs_is_ready() {
    return g_ready;
}

void tarfs_ls() {
    if (!g_ready) {
        kprint("ls: filesystem not ready\n");
        return;
    }

    size_t off = 0;
    size_t count = 0;
    while (true) {
        const TarHeader* hdr = nullptr;
        const uint8_t* data = nullptr;
        size_t size = 0;
        if (!tar_walk_next(&off, &hdr, &data, &size)) break;
        (void)data;
        (void)size;

        char path[256];
        tar_build_path(hdr, path, sizeof(path));
        const char* shown = skip_dot_slash(path);
        if (shown[0] == '\0') continue;

        kprint(shown);
        if (hdr->typeflag == '5') {
            size_t shown_len = str_len(shown);
            if (shown_len == 0 || shown[shown_len - 1] != '/') {
                kprint("/");
            }
        }
        kprint("\n");
        count++;
    }

    if (count == 0) {
        kprint("(empty)\n");
    }
}

bool tarfs_cat(const char* path) {
    if (!g_ready) {
        kprint("cat: filesystem not ready\n");
        return false;
    }
    if (!path || path[0] == '\0') {
        kprint("cat: missing file operand\n");
        return false;
    }

    const char* wanted = skip_dot_slash(path);
    size_t off = 0;
    while (true) {
        const TarHeader* hdr = nullptr;
        const uint8_t* data = nullptr;
        size_t size = 0;
        if (!tar_walk_next(&off, &hdr, &data, &size)) break;

        char full_path[256];
        tar_build_path(hdr, full_path, sizeof(full_path));
        const char* current = skip_dot_slash(full_path);

        if (!path_eq_normalized(current, wanted)) continue;

        if (hdr->typeflag == '5') {
            kprint("cat: ");
            kprint(path);
            kprint(": is a directory\n");
            return false;
        }

        for (size_t i = 0; i < size; i++) {
            char c = (char)data[i];
            console.write_char(c);
            Serial::write_char(c);
        }
        if (size == 0 || data[size - 1] != '\n') {
            kprint("\n");
        }
        return true;
    }

    kprint("cat: ");
    kprint(path);
    kprint(": no such file\n");
    return false;
}

void tarfs_self_test() {
    kprint("[TARFS TEST] ls\n");
    tarfs_ls();

    kprint("[TARFS TEST] cat hello.txt\n");
    tarfs_cat("hello.txt");

    kprint("[TARFS TEST] cat docs/guide.txt\n");
    tarfs_cat("docs/guide.txt");

    kprint("[TARFS TEST] cat missing.txt\n");
    tarfs_cat("missing.txt");

    kprint("[TARFS TEST] cat docs\n");
    tarfs_cat("docs");
}
