#include <nodePath.h>

void remove_children_except(const NodePath& parent, const std::vector<NodePath>& exceptions) {
    NodePathCollection children = parent.get_children();
    for (int i = 0; i < children.get_num_paths(); ++i) {
        NodePath& child = children[i];
        bool should_keep = false;

        for (const NodePath& exception : exceptions) {
            if (child == exception) {
                should_keep = true;
                break;
            }
        }

        if (!should_keep) {
            child.remove_node();
        }
    }
}
