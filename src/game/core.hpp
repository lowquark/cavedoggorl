#ifndef GAME_CORE_HPP
#define GAME_CORE_HPP

#include <cassert>
#include <algorithm>
#include <limits>
#include <vector>
#include <string>
#include <sstream>
#include <map>

namespace game {
  typedef unsigned long Id;

  class Part {
    // I don't like doing it but it works
    // also fuck dynamic_cast
    unsigned int _class_id;
    public:
    virtual ~Part() = default;
    Part(unsigned int _class_id) : _class_id(_class_id) {}
    unsigned int class_id() const { return _class_id; }

    virtual std::string serialize() const { return std::string(); }
  };

  class Object {
    public:
    bool add_part(Part * part) {
      unsigned int class_id = part->class_id();
      auto id_it = std::lower_bound(part_ids.begin(), part_ids.end(), class_id);
      auto parts_it = parts.begin() + (id_it - part_ids.begin());

      if(id_it == part_ids.end() || *id_it != class_id) {
        part_ids.insert(id_it, class_id);
        parts.insert(parts_it, part);
        return true;
      }
      return false;
    }
    Part * remove_part(unsigned int class_id) {
      auto id_it = std::lower_bound(part_ids.begin(), part_ids.end(), class_id);
      auto parts_it = parts.begin() + (id_it - part_ids.begin());

      if(id_it != part_ids.end() && *id_it == class_id) {
        Part * part = *parts_it;
        part_ids.erase(id_it);
        parts.erase(parts_it);
        return part;
      } else {
        return nullptr;
      }
    }
    bool has_part(unsigned int class_id) const {
      auto id_it = std::lower_bound(part_ids.begin(), part_ids.end(), class_id);

      return id_it != part_ids.end() && *id_it == class_id;
    }
    bool has_all(const std::vector<unsigned int> & class_ids) const {
      // Can be made faster
      for(auto & id : class_ids) {
        if(!has_part(id)) {
          return false;
        }
      }
      return true;
    }
    bool has_one(const std::vector<unsigned int> & class_ids) const {
      // Can be made faster
      for(auto & id : class_ids) {
        if(has_part(id)) {
          return true;
        }
      }
      return false;
    }
    Part * part(unsigned int class_id) const {
      auto id_it = std::lower_bound(part_ids.begin(), part_ids.end(), class_id);
      auto parts_it = parts.begin() + (id_it - part_ids.begin());

      if(id_it != part_ids.end() && *id_it == class_id) {
        return *parts_it;
      } else {
        return nullptr;
      }
    }

    /*
    void handle_event(unsigned int class_id, Event * event) const {
      for(auto & p : parts) {
        if(p) {
          p->handle_event(class_id, event);
        }
      }
    }
    */

    std::vector<std::string> serialize() const {
      std::vector<std::string> st;
      for(auto & p : parts) {
        if(p) {
          st.push_back(p->serialize());
        }
      }
      return st;
    }

    void clear() {
      part_ids.clear();
      parts.clear();
    }
    bool empty() {
      assert(part_ids.size() == parts.size());
      return part_ids.empty();
    }

    private:
    std::vector<unsigned int> part_ids;
    std::vector<Part *> parts;

    friend class Universe;
  };

  class Universe;

  class System {
    public:
    virtual ~System() = default;
    virtual void operator()(Universe & u, unsigned int oid) {}
  };

  class BasePartFactory {
    public:
    virtual ~BasePartFactory() = default;
    virtual Part * create(const std::string & data) { return nullptr; };
    virtual void destroy(Part * p) {};
  };

  class ObjectHandle {
    unsigned int _id;

    public:
    ObjectHandle()
      : _id(0) {}
    ObjectHandle(unsigned int _id)
      : _id(_id) {}

    bool operator==(ObjectHandle other) const {
      return _id == other._id;
    }
    operator bool() const {
      return _id != 0;
    }
    unsigned int id() const {
      return _id;
    }
    std::string str() const {
      if(_id) {
        std::stringstream ss;
        ss << "e.";
        ss << _id;
        return ss.str();
      } else {
        return "e.null";
      }
    }
  };
  //typedef unsigned int ObjectHandle;

  class Universe {
    BasePartFactory & pf;

    std::map<unsigned int, Object> objects2;

    unsigned int last_handle_id = 0;

    public:
    Universe(BasePartFactory & pf) : pf(pf) {}
    ~Universe() {
      for(auto & kvpair : objects2) {
        deinit_object(kvpair.second);
      }
    }

    ObjectHandle create_object() {
      return create_object({});
    }
    ObjectHandle create_object(const std::vector<std::string> & part_data) {
      if(last_handle_id < std::numeric_limits<decltype(last_handle_id)>::max()) {
        last_handle_id ++;

        objects2[last_handle_id] = new_object(part_data);

        return ObjectHandle(last_handle_id);
      } else {
        return ObjectHandle();
      }
    }
    void destroy_object(ObjectHandle obj) {
      auto it = objects2.find(obj.id());

      if(it != objects2.end()) {
        auto & o = it->second;
        deinit_object(o);
        objects2.erase(it);
      }
    }

    bool create_part(ObjectHandle obj, const std::string & data) {
      auto it = objects2.find(obj.id());

      if(it != objects2.end()) {
        auto & o = it->second;
        try_add(o, data);
        return true;
      }
      return false;
    }
    bool destroy_part(ObjectHandle obj, unsigned int part_class_id) {
      auto it = objects2.find(obj.id());

      if(it != objects2.end()) {
        auto & o = it->second;
        return o.remove_part(part_class_id);
      }
      return false;
    }
    Part * part(ObjectHandle obj, unsigned int part_class_id) const {
      auto it = objects2.find(obj.id());

      if(it != objects2.end()) {
        auto & o = it->second;
        return o.part(part_class_id);
      }
      return nullptr;
    }

    template <typename Fn>
    void for_all_with(const std::vector<unsigned int> & part_class_ids, Fn fn) {
      for(auto & kvpair : objects2) {
        if(kvpair.second.has_all(part_class_ids)) {
          fn(*this, ObjectHandle(kvpair.first));
        }
      }
    }
    template <typename Fn>
    void for_all_with(const std::vector<unsigned int> & part_class_ids, Fn fn) const {
      for(auto & kvpair : objects2) {
        if(kvpair.second.has_all(part_class_ids)) {
          fn(*this, ObjectHandle(kvpair.first));
        }
      }
    }

    bool has_any_with(const std::vector<unsigned int> & part_class_ids) const {
      for(auto & kvpair : objects2) {
        if(kvpair.second.has_all(part_class_ids)) {
          return true;
        }
      }
      return false;
    }

    std::vector<std::string> serialize(ObjectHandle obj) const {
      auto it = objects2.find(obj.id());

      if(it != objects2.end()) {
        auto & o = it->second;
        return o.serialize();
      }
      return std::vector<std::string>();
    }

    private:
    void try_add(Object & o, const std::string & data) {
      Part * p = pf.create(data);
      if(p) {
        Part * old = o.part(p->class_id());
        if(old) {
          o.remove_part(old->class_id());
          pf.destroy(old);
        }
        o.add_part(p);
      }
    }
    Object new_object(const std::vector<std::string> & part_data) {
      Object o;

      for(auto & data : part_data) {
        try_add(o, data);
      }

      return o;
    }
    void deinit_object(Object & o) {
      for(auto & p : o.parts) {
        pf.destroy(p);
      }
      o.clear();
    }
  };

  class PartClass {
    public:
    PartClass() : _id(next_id ++) {}
    PartClass(PartClass & other) = delete;
    PartClass & operator=(PartClass & rhs) = delete;

    unsigned int id() const { return _id; }
    operator unsigned int() const {
      return id();
    }

    private:
    unsigned int _id = 0;
    static unsigned int next_id;
  };

  // auxilliary
  template <typename T>
  class PartHelper : public Part {
    public:

    static const PartClass part_class;

    PartHelper() : Part(part_class) {}
  };
  template <typename T>
  const PartClass PartHelper<T>::part_class;

  template <typename T>
  static T * get_part(const Universe & u, ObjectHandle obj) {
    return static_cast<T *>(u.part(obj, T::part_class));
  }
  template <typename T>
  T * part_cast(Part * p) {
    if(p->class_id() == T::part_class) {
      return static_cast<T *>(p);
    }
    return nullptr;
  }
}

#endif
