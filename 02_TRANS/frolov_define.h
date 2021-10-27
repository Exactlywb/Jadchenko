#ifndef FROLOV_DEFINE__
#define FROLOV_DEFINE__

#define FUNCTION_ASSERT(condition, toDo, err)   do {                            \
                                                                                \
                                                    if (condition) {            \
                                                                                \
                                                        toDo                    \
                                                        return err;             \
                                                                                \
                                                    }                           \
                                                                                \
                                                } while (0)

#define IS_NULL(ptr) (!ptr)

#define FOREVER for(;;)

#endif
