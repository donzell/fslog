#ifndef _INITIALIZER_H_
#define _INITIALIZER_H_

class Initializer {
 public:
  typedef void (*void_function)(void);
  Initializer(const char*, void_function f) {
    f();
  }
};

#define REGISTER_MODULE_INITIALIZER(name, body)                 \
  namespace {                                                   \
    static void init_module_##name () { body; }          \
    Initializer initializer_module_##name(#name,   \
            init_module_##name);                         \
  }


#endif /* _INITIALIZER_H_ */
