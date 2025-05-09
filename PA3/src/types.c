//
// Created by Brandon Howe on 2/10/25.
//

#include "types.h"

#include <alloca.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bh_str bh_str_from_cstr(const char* cstr)
{
    return (bh_str){ .buf = cstr, .len = strlen(cstr) };
}

bh_str bh_str_alloc_cstr(bh_allocator allocator, const char* cstr)
{
    char* buf = bh_alloc(allocator, strlen(cstr));
    strncpy(buf, cstr, strlen(cstr));
    return (bh_str){ .buf = buf, .len = strlen(cstr) };
}

int8_t bh_str_equal(const bh_str str1, const bh_str str2)
{
    if (str1.len != str2.len) return 0;
    for (int i = 0; i < str1.len; i++)
    {
        if (str1.buf[i] != str2.buf[i])
        {
            return 0;
        }
    }
    return 1;
}

int8_t bh_str_cmp(const bh_str str1, const bh_str str2)
{
    if (bh_str_equal(str1, str2)) return 0;
    int smaller_len = str1.len < str2.len ? str1.len : str2.len;
    for (int i = 0; i < smaller_len; i++)
    {
        if (str1.buf[i] < str2.buf[i]) return -1;
        if (str1.buf[i] > str2.buf[i]) return 1;
    }
    return str1.len < str2.len ? -1 : 1;
}

bh_str until_newline(const bh_str str)
{
    for (int i = 0; i < str.len; i++)
    {
        if (str.buf[i] == '\n')
        {
            return (bh_str){ .buf = str.buf, .len = i };
        }
    }
    return str;
}

bh_str eat_until_newline(bh_str* str)
{
    bh_str res = { 0 };
    for (int i = 0; i < str->len; i++)
    {
        if (str->buf[i] == '\n')
        {
            res = (bh_str){ .buf = str->buf, .len = i };
            str->buf += i + 1;
            str->len -= i + 1;
            return res;
        }
    }
    res = (bh_str){ .buf = str->buf, .len = str->len };
    str->buf += str->len;
    str->len -= str->len;
    return res;
}

int32_t eat_uint_until_newline(bh_str* str)
{
    bh_str res = (bh_str){ 0 };
    for (int i = 0; i < str->len; i++)
    {
        if (str->buf[i] == '\n')
        {
            res = (bh_str){ .buf = str->buf, .len = i };
            str->buf += i + 1;
            str->len -= i + 1;
            return uint_from_str(res);
        }
    }
    res = (bh_str){ .buf = str->buf, .len = str->len };
    str->buf += str->len;
    str->len -= str->len;
    return uint_from_str(res);
}

int32_t uint_from_str(const bh_str str)
{
    int32_t res = 0;
    for (int i = 0; i < str.len; i++)
    {
        if (str.buf[i] >= '0' && str.buf[i] <= '9')
        {
            res *= 10;
            res += str.buf[i] - '0';
        }
        else
        {
            return -1;
        }
    }
    return res;
}

bh_str_buf bh_str_buf_init(bh_allocator allocator, uint64_t capacity)
{
    bh_str_buf str_buf = (bh_str_buf){
        .buf = bh_alloc(allocator, capacity),
        .len = 0,
        .cap = capacity,
        .allocator = allocator
    };
    return str_buf;
}

void bh_str_buf_append(bh_str_buf* str_buf, bh_str str)
{
    if (str_buf->len + str.len + 1 >= str_buf->cap)
    {
        bh_str_buf_reserve(str_buf, str_buf->cap * 2);
    }
    memcpy(str_buf->buf + str_buf->len, str.buf, str.len);
    str_buf->len += str.len;
}

void bh_str_buf_append_str_buf(bh_str_buf* str_buf, bh_str_buf str_buf_2)
{
    if (str_buf->len + str_buf_2.len + 1 >= str_buf->cap)
    {
        bh_str_buf_reserve(str_buf, str_buf->cap * 2);
    }
    memcpy(str_buf->buf + str_buf->len, str_buf_2.buf, str_buf_2.len);
    str_buf->len += str_buf_2.len;
}

void bh_str_buf_append_format(bh_str_buf* buf, const char* format, ...)
{
    va_list va = { 0 };
    va_start(va, format);

    uint64_t append_size = vsnprintf(NULL, 0, format, va);
    if (buf->len + append_size + 1 >= buf->cap)
    {
        bh_str_buf_reserve(buf, buf->cap * 2);
    }

    va_end(va);
    va_start(va, format);

    vsnprintf(&buf->buf[buf->len], buf->cap - buf->len + 1, format, va);

    buf->len += append_size;

    va_end(va);
}

void bh_str_buf_append_lit(bh_str_buf* str_buf, const char* cstr)
{
    bh_str_buf_append(str_buf, bh_str_from_cstr(cstr));
}

void bh_str_buf_reserve(bh_str_buf* str_buf, uint64_t capacity)
{
    if (capacity < str_buf->cap) return;
    str_buf->buf = bh_realloc(str_buf->allocator, str_buf->buf, capacity);
    str_buf->cap = capacity;
}

void bh_str_buf_clear(bh_str_buf* str_buf)
{
    str_buf->len = 0;
}

void bh_str_buf_deinit(bh_str_buf* str_buf)
{
    bh_free(str_buf->allocator, str_buf->buf);
}

bh_str read_file_text(const char* file_name)
{
    FILE *file = fopen(file_name, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return (bh_str){ 0 };
    }

    // Seek to the end to determine the file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    // Allocate memory to hold the file content
    char* buffer = bh_alloc(GPA, fileSize);
    if (buffer == NULL) {
        perror("Failed to allocate memory");
        fclose(file);
        return (bh_str){ 0 };
    }

    // Read the file into the buffer
    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead != fileSize) {
        perror("Failed to read the file");
        free(buffer);
        fclose(file);
        return (bh_str){ 0 };
    }

    //fclose(file);
    return (bh_str){ .buf = buffer, .len = bytesRead };
}