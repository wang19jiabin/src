#include <iostream>
#include <map>
#include <set>

using namespace std;

class Map {
public:
  void insert(int parent, int child) {
    _parentChildren[parent].insert(child);
    _childParent[child] = parent;
  }

  set<int> findChildren(int parent) const {
    auto children = _parentChildren.find(parent);
    if (children == _parentChildren.end())
      return {};

    return children->second;
  }

  int findParent(int child) const {
    auto parent = _childParent.find(child);
    if (parent == _childParent.end())
      return -1;

    return parent->second;
  }

  void print() const {
    for (const auto &children : _parentChildren)
      for (auto child : children.second)
        cout << children.first << " - " << child << endl;

    cout << endl;
    for (const auto &parent : _childParent)
      cout << parent.first << " - " << parent.second << endl;
  }

private:
  map<int, set<int>> _parentChildren;
  map<int, int> _childParent;
};

int main() {
  Map map;
  map.insert(0, 00);
  map.insert(0, 00);
  map.insert(0, 00);
  map.insert(1, 11);
  map.insert(1, 12);
  map.insert(1, 13);
  map.insert(2, 21);
  map.insert(2, 22);
  map.insert(2, 23);
  map.print();

  auto children = map.findChildren(1);
  for (auto child : children)
    cout << child << endl;

  auto parent = map.findParent(23);
  cout << parent << endl;
}
