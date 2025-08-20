#include <fontPool.h>
#include <textNode.h>
#include "runtimeScript.hpp"

class TextNodeDemo : public RuntimeScript {
public:
    TextNodeDemo(Demon& demon) : RuntimeScript(demon) {        
        // === Create a TextNode ===
        PT(TextNode) text_node = new TextNode("demo_text");
        
        // === Text Alignment ===
        text_node->set_align(TextNode::A_center);
        
        // === Other align types ===
        // text_node->set_align(TextNode::A_left);
        // text_node->set_align(TextNode::A_right);

        // === Set Text and Text Color ===
        text_node->set_text("Every day in every way I am getting better and better."); // Set text
        text_node->set_text_color(LColor(1.f, 1.f, 1.f, 1.f)); // Change text color

        // === Optional Font ===
        // You can load a custom font if needed
        // text_node->set_font(FontPool::load_font("custom.ttf"));

        // === Word Wrapping ===
        text_node->set_wordwrap(15.0f);  // Wrap text at 15 character units
        
        // === Slant ===
        // Slant can be used to give an effect similar to italicizing.
        // The parameter value is 0.0 for no slant, or 1.0 for a 45-degree rightward slant
        // You can also use a negative number to give a reverse slant.
        text_node->set_slant(0);
        
        // === Shadow ===
        text_node->set_shadow_color(LColor(0.0f, 0.0f, 0.2f, 1.0f));
        text_node->set_shadow(0.025f, 0.025f); // Offset shadow

        // === Background Card (Text box) ===
        text_node->set_card_color(LColor(0.05f, 0.1f, 0.2f, 0.6f));  // Semi-transparent black
        text_node->set_card_as_margin(0.1f, 0.1f, 0.1f, 0.1f);
        
        // === Frame Border ===
        text_node->set_frame_color(LColor(0.0f, 1.0f, 0.8f, 1.0f));
        text_node->set_frame_as_margin(0.1f, 0.1f, 0.1f, 0.1f);
        text_node->set_frame_line_width(1.5f);
        
        /*
            "text_node->set_card_as_margin" and "text_node->set_frame_as_margin" and specify
            the distance to extend the card beyond the left, right, bottom, and top edges
            of the text, respectively.
        */
        
        // === For 3D rendering ===
        // If the text is to be visible in the 3d world, that is, parented to render instead 
        // of to render2D or aspect2D, then you may observe z-fighting, or flashing, between 
        // the text and the card.
        // To avoid this, call text.set_card_decal(True). 
        // Not necessary if text is going to be rendered in 2D.
        text_node->set_card_decal(false);
        
        // === Wrap text node into a NodePath and put it in scene graph ===
        NodePath text_np = NodePath(text_node->generate());
        text_np.reparent_to(game.aspect2D);
        
        // NodePath operations as usual
        text_np.set_scale(0.2f);      // Uniform scale
        text_np.set_pos(0.f, 0.f, 0.35f); // X, Y, Z position in aspect2D
        text_np.set_hpr(0.f, 0.f, 0.f);     // Rotate slightly for effect
        
        // --------------------------------------------------------------------
        // Other useful options
        
        // === Small Caps ===
        // In this mode, instead of rendering lowercase letters, the TextNode renders capital
        // letters that are a bit smaller than the true capital letters.
        // This is an especially useful feature if your font of choice doesn’t happen to
        // include lowercase letters.
        
        // "text_node->set_small_caps_scale" spcify the relative scale of the "lowercase" letters
        
        // text_node.set_small_caps(True);
        // text_node->set_small_caps_scale(0.8f);
    }
};

REGISTER_SCRIPT(TextNodeDemo)
