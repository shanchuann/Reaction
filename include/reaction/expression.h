#include <tuple>
#include "reaction/resource.h"

namespace reaction {
    template<typename Fun, typename... Args>
    class Expression : public Resource<> 
    {
    public:

    private:
        Fun m_fun;
        std::tuple<Args...> m_args;
    };
    template<typename Type>
    class Expression<Type> : Resource<Type> {};
} // namespace reaction
