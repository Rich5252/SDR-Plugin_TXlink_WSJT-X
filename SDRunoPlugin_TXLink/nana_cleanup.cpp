#include <nana/gui/detail/bedrock.hpp>
#include "SDRunoPlugin_TXLinkUi.h" // Or wherever the get_bedrock_instance declaration is

nana::detail::bedrock& get_bedrock_instance() {
    return nana::detail::bedrock::instance();
}