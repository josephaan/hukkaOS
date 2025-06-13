#include <string.h>

// Calculates the length of a string

size_t strlen(const char * str) {
	size_t len = 0;
	while (*str++){
		len++;
	}
	return len;
}

// Finds the first occurence of a substring needle in the string haystack
char * strstr(const char * haystack, const char * needle) {
	if (!*needle) {
		return (char *)haystack;
	}

	for (; *haystack; haystack++) {
		const char *h = haystack;
		const char *n = needle;

		while (*h && *n && *h == *n) {
			h++;
			n++;
		}

		if (!*n) {
			return (char *)haystack;
		}
	}
	return NULL;
}

// Compare two strings
int strcmp(const char *str_1, const char *str_2) {
	while (*str_1 && (*str_1 == *str_2)) {
		str_1++;
		str_2++;
	}
	return *(unsigned char *)str_1 - *(unsigned char *)str_2;
}

// Copy a string
char *strcpy(char *dest, const char *src){
	char *d = dest;
	while ((*d++ = *src++)) {
		// Copy the string
	}

	return dest;
}


// Concenate two strings
char *strcat(char *dest, const char * src) {
	char *d = dest;
	while (*d) {
		d++;
	}
	while ((*d++ = *src++)) {
		// Concenate the strings;
	}
	return dest;
}

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}


int strncmp(const char *s1, const char *s2, register size_t n) {
  register unsigned char u1, u2;

  while (n-- > 0)
    {
      u1 = (unsigned char) *s1++;
      u2 = (unsigned char) *s2++;
      if (u1 != u2)
	return u1 - u2;
      if (u1 == '\0')
	return 0;
    }
  return 0;
}


char *strncpy(char * restrict s1, const char * restrict s2, size_t n){

    char *d = s1;
    size_t i = 0;
    int length_of_source = strlen(s2);
    if(length_of_source < n){
        int remaining_chars = n - length_of_source;
        for(int j = 0; j < length_of_source; j++) {
            *d++ = *s2++;
        }

        for(int k = 0; k < remaining_chars; k++) {
            *d++ = 0;
        }


    } else{
        while ((*d++ = *s2++) && (i < n)) {
            // Copy the string
            i++;
        }
    }

	return s1;

}