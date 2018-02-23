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
constexpr bool do_all_derive()
{
    if constexpr (sizeof...(r) != 0)
    {
        return std::is_base_of<target, f>::value && do_all_derive<target, r...>();
    }
    else
    {
        return std::is_base_of<target, f>::value;
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

template<typename subset_first, typename...subset>
struct is_subset
{
    template<typename...superset>
    static constexpr bool of()
    {
        if constexpr(sizeof...(subset) == 0)
        {
            return is_one_of<subset_first,superset...>();
        }
        else
        {
            return is_one_of<subset_first,superset...>() && is_subset<subset...>::template of<superset...>();
        }
    }
};

}

template<typename base, typename...derived>
class algebraic
{
    template<typename obase,typename...oderived>
    friend class algebraic;

    using dt = stack_virt<base, assertion_utilities::largest_class<derived...>()>;

public:

    template<typename target,typename...arg_types>
    static algebraic<base,derived...> make(arg_types&&...args)
    {
        static_assert(assertion_utilities::is_one_of<target,derived...>(),"target type must be one of the listed derived classes");
        return algebraic{dt::template make<target>(std::forward<arg_types>(args)...)};
    }

    algebraic(algebraic const& a) = delete;
    
    void operator=(algebraic const& a) = delete;

    template<typename oldbase,typename...oldderived>
    algebraic<base,derived...>(algebraic<oldbase,oldderived...>&& a) :
        algebraic(dt(std::move(a.data)))
    {
        static_assert(assertion_utilities::is_subset<oldderived...>::template of<derived...>(),"cannot construct an algebraic<ts...> from an algebraic that could contain a type other than the ones in ts...");
    }


    template<typename oldbase,typename...oldderived>
    void operator=(algebraic<oldbase,oldderived...>&& a)
    {
        static_assert(assertion_utilities::is_subset<oldderived...>::template of<derived...>(),"cannot construct an algebraic<ts...> from an algebraic that could contain a type other than the ones in ts...");
        operator=(dt(std::move(a.data)));
    }


    

private:


    algebraic(dt&& a) :
        data(std::move(a))
    {
        static_assert(assertion_utilities::do_all_derive<base,derived...>(),"all of derived must derive from base");
    }

    void operator=(dt&& a)
    {
        data = std::move(a);
    }

    dt data;



};