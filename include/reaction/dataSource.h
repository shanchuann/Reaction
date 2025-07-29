#include "reaction/expression.h"

namespace reaction{
    template<typename T, typename... Args>
    class DataSource : public Expression<T, Args...>
    {
    public:
        auto get() const {

        }
    };

    template<typename T>
    auto var(T &&t){
        return DataSource<T>(std::forward<T>(t));
    }

    template<typename Func, typename... Args>
    auto calc(Func &&func, Args &&...args) {
        return DataSource<Func, Args...>(std::forward<Func>(func), std::forward<Args>(args)...);
    }
}