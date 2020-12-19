#ifndef MICROSHA_MATCHER_H
#define MICROSHA_MATCHER_H

class Matcher {
public:
  Matcher(const char* name, const char* mask) : name(name), mask(mask) {}

  bool match() {
    if (!try_partial_match())
      return false;

    while (*mask == '*') {
      ++mask;
      while (!try_partial_match() && *name != '\0')
        ++name;
    }

    return is_full_match();
  }

private:
  bool is_full_match() const { return *name == '\0' && *mask == '\0'; }

  bool patrial_match() {
    while (*name != '\0' && (*name == *mask || *mask == '?')) {
      ++name;
      ++mask;
    }

    return is_full_match() || *mask == '*';
  }

  bool try_partial_match() {
    auto tmp = *this;
    if (tmp.patrial_match()) {
      *this = tmp;
      return true;
    }
    return false;
  }

  const char* name;
  const char* mask;
};

#endif //MICROSHA_MATCHER_H
