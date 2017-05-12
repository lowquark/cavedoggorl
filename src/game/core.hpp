#ifndef GAME_CORE_HPP
#define GAME_CORE_HPP

#include <cassert>
#include <algorithm>
#include <limits>
#include <vector>
#include <string>

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
    bool has_part(unsigned int class_id) {
      auto id_it = std::lower_bound(part_ids.begin(), part_ids.end(), class_id);

      return id_it != part_ids.end() && *id_it == class_id;
    }
    bool has_all(const std::vector<unsigned int> & class_ids) {
      // Can be made faster
      for(auto & id : class_ids) {
        if(!has_part(id)) {
          return false;
        }
      }
      return true;
    }
    bool has_one(const std::vector<unsigned int> & class_ids) {
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

  struct ObjectHandle {
    unsigned int id;
    unsigned int key;
  };

  class Universe {
    BasePartFactory & pf;

    std::vector<Object> objects;
    std::vector<unsigned int> keys;
    std::vector<unsigned int> destroyed_object_ids;

    public:
    Universe(BasePartFactory & pf) : pf(pf) {}
    ~Universe() {
      for(auto & o : objects) {
        deinit_object(o);
      }
    }

    inline bool is_id_valid(ObjectHandle obj) const {
      assert(objects.size() == keys.size());
      if(obj.id == 0) { return false; }
      if(obj.id > objects.size()) { return false; }
      return obj.key == keys[obj.id-1];
    }
    inline bool is_id_valid(unsigned int id) const {
      return id != 0 && id <= objects.size();
    }

    unsigned int create_object() {
      return create_object({});
    }
    unsigned int create_object(const std::vector<std::string> & part_data) {
      if(destroyed_object_ids.empty()) {
        if(objects.size() < std::numeric_limits<unsigned int>::max()) {
          objects.push_back(new_object(part_data));
          return objects.size();
        } else {
          return 0;
        }
      } else {
        unsigned int new_id = destroyed_object_ids.back();
        destroyed_object_ids.pop_back();

        objects[new_id-1] = new_object(part_data);
        return new_id;
      }
    }
    void destroy_object(unsigned int id) {
      if(is_id_valid(id)) {
        deinit_object(objects[id-1]);

        destroyed_object_ids.push_back(id);
      }
    }

    bool create_part(unsigned int id, const std::string & data) {
      if(is_id_valid(id)) {
        auto & o = objects[id-1];
        return try_add(o, data);
      }
      return false;
    }
    bool destroy_part(unsigned int id, unsigned int part_class_id) {
      if(is_id_valid(id)) {
        auto & o = objects[id-1];
        return o.remove_part(part_class_id);
      }
      return false;
    }
    Part * part(unsigned int oid, unsigned int part_class_id) const {
      if(is_id_valid(oid)) {
        auto & o = objects[oid-1];
        return o.part(part_class_id);
      }
      return nullptr;
    }

    template <typename Fn>
    void for_all_with(const std::vector<unsigned int> & part_class_ids, Fn fn) {
      unsigned int id = 1;
      for(auto & o : objects) {
        if(o.has_all(part_class_ids)) {
          fn(*this, id);
        }
        id ++;
      }
    }

    bool has_any_with(const std::vector<unsigned int> & part_class_ids) {
      unsigned int id = 1;
      for(auto & o : objects) {
        if(o.has_all(part_class_ids)) {
          return true;
        }
        id ++;
      }
      return false;
    }

    std::vector<std::string> serialize(unsigned int oid) const {
      if(is_id_valid(oid)) {
        auto & o = objects[oid-1];
        return o.serialize();
      }
      return std::vector<std::string>();
    }

    private:
    bool try_add(Object & o, const std::string & data) {
      Part * p = pf.create(data);
      if(p) {
        if(o.add_part(p)) {
          return true;
        } else {
          pf.destroy(p);
        }
      }
      return false;
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
  static T * get_part(const Universe & u, unsigned int oid) {
    return static_cast<T *>(u.part(oid, T::part_class));
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
