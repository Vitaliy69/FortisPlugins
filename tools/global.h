#ifdef UNUSED
#elif __GNUC__ >= 3 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif 
