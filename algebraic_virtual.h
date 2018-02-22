#include<array>
#include<typeinfo>
#include"stack-based_virtual.h"

//just as a namespace
namespace assertion_utilities
{

template <typename target, typename f, typename...r>
constexpr bool is_one_of()
{
    if constexpr (sizeof...(r) != 0)
    {
        return std::is_same<target, f>::value || is_one_of<target, r...>();
    }
    else
    {
        return std::is_same<target, f>::value;
    }
}

template <typename target, typename f, typename... r>
void assert_all_derive()
{
    if constexpr (sizeof...(r) != 0)
    {
        static_assert(std::is_base_of<target, f>::value, "first template parameter must be the base class of all following template parameters");
        assert_all_derive<target, r...>();
    }
    else
    {
        static_assert(std::is_base_of<target, f>::value, "first template parameter must be the base class of all following template parameters");
    }
}

template <typename f, typename... r>
constexpr size_t largest_class()
{
    if constexpr (sizeof...(r) != 0)
    {
        constexpr size_t rs = largest_class<r...>();
        if constexpr (sizeof(f) > rs)
        {
            return sizeof(f);
        }
        else
        {
            return rs;
        }
    }
    else
    {
        return sizeof(f);
    }
}

}

template<typename base, typename...derived>
class algebraic
{

    using dt = stack_virt<base, assertion_utilities::largest_class<derived...>()>;

public:

    template<typename target,typename...arg_types>
    static algebraic<base,derived...> make(arg_types&&...args)
    {
        static_assert(assertion_utilities::is_one_of<target,derived...>(),"target type must be one of the listed derived classes");
        return algebraic{dt::make<target>(std::forward<arg_types>(args)...)};
    }

    algebraic(algebraic const& a) = delete;
    
    void operator=(algebraic const& a) = delete;

    algebraic<base,derived...>(algebraic<base,derived...>&& a) :
        algebraic(std::move(a.data))
    {

    }
    


    

private:


    algebraic(dt&& a) :
        data(std::move(a))
    {
        assertion_utilities::assert_all_derive<base,derived...>();
    }

    void operator=(dt&& a)
    {
        assertion_utilities::assert_all_derive<base,derived...>();
        data = std::move(a);
    }

    dt data;



};