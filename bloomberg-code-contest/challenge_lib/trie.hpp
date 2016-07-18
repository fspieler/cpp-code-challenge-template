#include <utility>
#include <algorithm>

Trie::Trie(TMap&& map) :
  _count(1),
  _empty_count(0),
  _full_count(0),
  _str(),
  _is_empty_word(false),
  _is_full_word(false),
  _map(map)
{}

namespace trie_helper {

Trie& move_suffix_to_intermediate_node(Trie& t, int index)
{
  int length = t._str.size() - index;
  if(length <= 0) return t;
  auto new_str = t._str.substr(index, length);
  auto new_map = std::move(t._map);
  t._str = t._str.substr(0,index);
  Trie& new_trie = t._map[new_str[0]];
  if(new_str.size() > 1)
  {
    auto substr = new_str.substr(1,new_str.size()-1);
    DEBUG(substr);
    new_trie.add_string(std::move(substr));
  }
  else if(new_str.size() == 1 && t._is_full_word)
  {
    new_trie._empty_count = t._full_count;
    new_trie._is_empty_word=true;
  }
  new_trie._count = t._count - 1;
  new_trie._map = std::move(new_map);
  return new_trie;
}

} //namespace trie_helper

Trie& Trie::add_string(const std::string& s)
{
  this->_count++;
  if(!s.size())
  {
    _is_empty_word=true;
    _empty_count++;
    return *this;
  }
  if(!_str.size())
  {
    if(_map.size() > 0)
    {
      _map[s[0]].add_string(s.substr(1,s.size()-1));
    }
    else
    {
      _str = s;
      _is_full_word=true;
      _full_count++;
    }
    return *this;
  }

  int common_index=-1;
  for
    (
      size_t i = 0;
      i < s.size() && i < _str.size();
      ++i
    )
  {
    if(s[i] == _str[i])
    {
      common_index=i;
    }
    else break;
  }
  
  if(s.size() == _str.size() && common_index == s.size()-1)
  {
    _full_count++;
    return *this;
  }

  if(common_index >= 0)
  {
    int start= common_index+1;
    int length1 = _str.size() - start;
    if(length1>=0) //original str longer than common
    {
      Trie& new_trie  = trie_helper::move_suffix_to_intermediate_node(*this,start);
      if(length1>1)
      {
        new_trie._is_full_word = _is_full_word;
        new_trie._full_count = _full_count;
      }
      _is_full_word = false;
      _full_count = 0;
    }
    int length2 = s.size() - start;
    if(length2 > 0) //new str longer than common
    {
      Trie& new_entry = _map[s[start]];
      std::string new_str = s.substr(start+1, length2-1);
      new_entry.add_string(new_str);
    }
    else
    {
      _is_full_word = true;
      _full_count++;
    }
    _str = _str.substr(0,start);
  }
  else
  {
    trie_helper::move_suffix_to_intermediate_node(*this,0);
    _map[s[0]].add_string(s.substr(1,s.size()-1));
  }

  return *this;
}

void Trie::next_letters(std::vector<char>& vc, const std::string& s) const
{
  if(s.size() < _str.size())
  {
    vc.push_back(_str[s.size()]);
    return;
  }
  else if(s.size() == _str.size())
  {
    for(auto& kv : _map)
    {
      vc.push_back(kv.first);
    }
    return;
  } else // s.size() > _str.size()
  {
    for(size_t i = 0; i < s.size() && i < _str.size(); ++i)
    {
      if(s[i] != _str[i]) return;
    }
    auto it = _map.find(s[_str.size()]);
    if(it == _map.end()) return;
    std::string new_str = s.substr(_str.size()+1, s.size()-_str.size());
    _map.at(s[_str.size()]).next_letters(vc, new_str);
  }
}

const Trie* Trie::_navigate_to_trie(const std::string& str, int& index) const
{
  if(!str.size())
  {
    return this;
  }
  for(size_t i = 0; i < _str.size(); ++i) {
    if(i >= str.size())
    {
      return this;
    }
    if(str[i] != _str[i]) return NULL;
  }
  if(str.size() == _str.size()) return this;
  auto it = _map.find(str[_str.size()]);
  if(it == _map.end()) return NULL;
  index += _str.size() + 1;
  return _map.at(str[_str.size()])._navigate_to_trie(str.substr(_str.size()+1,str.size()-_str.size()-1), index);
}

void Trie::_autocomplete_helper(std::vector<std::pair<std::string, int> >& wordCounts, const std::string& prefix, const Trie& t) const
{
  if(t._is_empty_word)
  {
    std::pair<std::string, int> p = std::pair<std::string, int>(prefix, t._empty_count);
    wordCounts.push_back(p);
  }
  if(t._is_full_word)
  {
    std::pair<std::string, int> p = std::pair<std::string, int>(prefix+t._str, t._full_count);
    wordCounts.push_back(p);
  }
  for(const auto& kv : t._map)
  {
    _autocomplete_helper(wordCounts, prefix+t._str+kv.first, kv.second);
  }
}

void Trie::autocomplete_words(std::vector<std::string>& v, const std::string& s, int limit) const
{
  std::vector<std::pair<std::string, int> > pair_vec;
  int index = 0;
  const Trie* t = _navigate_to_trie(s, index);
  if(!t) return;
  _autocomplete_helper(pair_vec, s.substr(0,index+1), *t);
  std::sort(pair_vec.begin(), pair_vec.end(), [](auto a, auto b) -> bool
      {
        return std::get<1>(a) > std::get<1>(b);
      }
  );
  for(auto e : pair_vec)
  {
    v.push_back(std::get<0>(e));
  }
}

std::ostream& Trie::print
  (
   std::ostream& os,
   bool indent,
   int level
  ) const
{
  std::string newline = indent ? "\n" : ", ";
  std::string outer_indent;
  int temp = level;
  while(temp-->0)
  {
    outer_indent+="\t";
  }
  std::string inner_indent = indent ? outer_indent + '\t' : "";
  os << "{ trie " << newline;

  os << inner_indent << "count: " << _count << newline;
  os << inner_indent << "is empty a word?: " << (_is_empty_word?"true":"false") << newline;
  if(_is_empty_word)
  {
    os << inner_indent << "empty word count: " << _empty_count << newline;
  }
  os << inner_indent << "is full a word?: " << (_is_full_word?"true":"false") << newline;
  if(_is_full_word)
  {
    os << inner_indent << "full word count: " << _full_count << newline;
  }
  if(_str.size() >= 1)
  {
    os << inner_indent << "base string: \"" << _str << '\"' << newline;
  }
  if(_map.size() >= 1)
  {
    for(auto &kv : _map)
    {
      os << inner_indent << kv.first << ": ";
      kv.second.print(os,indent,indent?level+1:0);
    }
  }
  os << outer_indent << "}" << newline;
  return os;
}