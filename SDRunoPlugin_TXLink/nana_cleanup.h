#pragma once

namespace nana {
    namespace detail {
        class bedrock;
    }
}
nana::detail::bedrock& get_bedrock_instance();
