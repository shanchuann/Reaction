#include <memory>

namespace reaction {
    template<typename Type>
    class Resource {
    public:
        
    private:
        std::unique_ptr<Type> m_value;
    };
}