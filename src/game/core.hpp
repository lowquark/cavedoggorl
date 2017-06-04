#ifndef GAME_CORE_HPP
#define GAME_CORE_HPP

#include <cassert>
#include <algorithm>
#include <vector>
#include <memory>
#include <map>

namespace nc {
  typedef unsigned int TypeId;

  class Event {
    public:
    Event(TypeId type_id) : _type_id(type_id) {}
    virtual ~Event() = default;

    TypeId type_id() const { return _type_id; }

    private:
    TypeId _type_id;
  };

  class Query {
    public:
    Query(TypeId type_id) : _type_id(type_id) {}
    virtual ~Query() = default;

    TypeId type_id() const { return _type_id; }
    void abort() { _abort_flag = true; }
    bool abort_flag() const { return _abort_flag; }

    private:
    TypeId _type_id;
    bool _abort_flag = false;
  };

  class Part {
    public:
    Part(TypeId type_id) : _type_id(type_id) {}
    virtual ~Part() = default;

    TypeId type_id() const { return _type_id; }

    virtual void notify(const Event & event) { }
    virtual void query(Query & query) const { }

    private:
    TypeId _type_id;
  };

  class Entity {
    public:
    void notify(const Event & event) const {
      for(auto & p : parts) {
        p->notify(event);
      }
    }
    bool query(Query & query) const {
      for(auto & p : parts) {
        if(query.abort_flag()) { return true; }
        p->query(query);
      }
      return false;
    }

    void add_part(std::unique_ptr<Part> && part) {
      for(auto & p : parts) {
        if(p->type_id() == part->type_id()) {
          p = std::move(part);
          return;
        }
      }
      parts.push_back(std::move(part));
    }
    void remove_part(TypeId type_id) {
      for(auto it = parts.begin() ;
          it != parts.end() ; ) {
        if((*it)->type_id() == type_id) {
          parts.erase(it);
          return;
        } else {
          it ++;
        }
      }
    }

    private:
    std::vector<std::unique_ptr<Part>> parts;

    friend class EntityManager;
  };

  struct Id {
    unsigned int idx = 0;
    unsigned int key = 0;

    bool operator<(const Id & other) const { return idx < other.idx; }
    operator bool() const {
      return key != 0;
    }
  };

  class IdReel {
    public:
    //const std::vector<Id> & valid() const { return _valid; }

    Id new_id() {
      if(_deleted.empty()) {
        Id id;
        id.idx = top_idx++;
        id.key = 1;
        _idxs.push_back(id.idx);
        _keys.push_back(id.key);
        return id;
      } else {
        Id id = _deleted.back();
        _deleted.pop_back();
        id.key ++;
        insert_id(id);
        return id;
      }
    }
    void delete_id(Id id) {
      auto idx_it = std::lower_bound(_idxs.begin(), _idxs.end(), id.idx);
      auto key_it = _keys.begin() + (idx_it - _idxs.begin());

      if(idx_it == _idxs.end()) { return; }
      //if(key_it == _keys.end()) { return; }
      if(*idx_it != id.idx) { return; }
      if(*key_it != id.key) { return; }

      _deleted.push_back(id);
      _idxs.erase(idx_it);
      _keys.erase(key_it);
    }

    bool is_valid(Id id) const {
      if(id.key == 0) { return false; }

      auto idx_it = std::lower_bound(_idxs.begin(), _idxs.end(), id.idx);
      auto key_it = _keys.begin() + (idx_it - _idxs.begin());

      if(idx_it == _idxs.end()) { return false; }
      //if(key_it == _keys.end()) { return false; }
      if(*idx_it != id.idx) { return false; }
      if(*key_it != id.key) { return false; }

      return true;
    }

    // TODO: Iteration

    private:
    void insert_id(Id id) {
      auto idx_it = std::lower_bound(_idxs.begin(), _idxs.end(), id.idx);
      auto key_it = _keys.begin() + (idx_it - _idxs.begin());

      assert(*idx_it != id.idx);

      _idxs.insert(idx_it, id.idx);
      _keys.insert(key_it, id.key);
    }

    unsigned int top_idx = 0;
    std::vector<unsigned int> _idxs;
    std::vector<unsigned int> _keys;
    std::vector<Id> _deleted;
  };

  template <typename T>
  class IdSet {
    public:
    Id create() {
      Id id = _reel.new_id();
      if(id.idx >= _values.size()) {
        _values.resize(id.idx + 1);
      }
      _values[id.idx] = T();
      return id;
    }
    void destroy(Id id) {
      _reel.delete_id(id);
    }

    const T * operator[](Id id) const {
      if(_reel.is_valid(id)) {
        return &_values[id.idx];
      } else {
        return nullptr;
      }
    }
    T * operator[](Id id) {
      if(_reel.is_valid(id)) {
        return &_values[id.idx];
      } else {
        return nullptr;
      }
    }

    private:
    IdReel _reel;
    std::vector<T> _values;
  };
}

#endif
