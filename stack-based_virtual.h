#include<array>
#include<assert.h>

template<typename base,size_t cap>
class stack_virt
{
    template<typename fbase,size_t fcap>
    friend class stack_virt;
public:
    
    template<typename target_type = base,typename...arg_types>
    static stack_virt<base,cap> make(arg_types&&...args)
    {
        static_assert(std::is_base_of<base,target_type>::value || std::is_same<base,target_type>::value,"stack_virt<b,c> can be made with the construction of any class that derives from base, or is base itself");
        stack_virt<base,cap> ret;
        ret.set_state_constructed();
        new (ret.get()) target_type(std::forward<arg_types>(args)...);
        return ret;
    }

    stack_virt(stack_virt const& a) = delete;

    void operator=(stack_virt const& a) = delete;

    template<typename b, size_t c>
    stack_virt(stack_virt<b,c>&& a)
    {
        static_assert(cap >= sizeof(base), "cap must be larger than the size of the base class");
        static_assert((std::is_base_of<base,b>::value || std::is_same<base,b>::value) && c<=cap, "to construct stack_virt<xb,xc> from stack_virt<yb,yc>&&, yb must be the same as or derive from xb, and yc must be less than or equal to xc");
        set_state_constructed();
        for(int i = 0; i != a.data.size(); ++i)
        {
            data[i] = a.data[i];
        }
        a.set_state_not_constructed();
    }

    template<typename b, size_t c>
    void operator=(stack_virt<b,c>&& a)
    {
        get()->~base();
        static_assert((std::is_base_of<base,b>::value || std::is_same<base,b>::value) && c<=cap, "to assign stack_virt<xb,xc>&& to stack_virt<yb,yc>, xb must be the same as or derive from yb, and xc must be less than or equal to yc");
        for(int i = 0; i != a.data.size(); ++i)
        {
            data[i] = a.data[i];
        }
        a.set_state_not_constructed();
    }

    template<typename newbase>
    stack_virt<newbase,cap> upcast() &&
    {
        static_assert((std::is_base_of<newbase,base>::value || std::is_same<newbase,base>::value), "to upcast a stack_virt<xb,xc>&& into a stack_virt<yb,yc>, xb must derive from yb");
        stack_virt ret;
        ret.set_state_constructed();
        set_state_not_constructed();
        return stack_virt<newbase,cap>{std::move(*this)};
    }

    template<size_t newcap>
    stack_virt<base,newcap> enlarge() &&
    {
        static_assert(newcap>=cap,"can not enlarge to a smaller capacity");
        stack_virt<base,newcap> ret;
        ret.set_state_constructed();
        for(int i = 0; i!=data.size(); ++i)
        {
            ret.data[i] = data[i];
        }
        set_state_not_constructed();
        return ret;
    }

    base* get()
    {
        if(!check_state_whether_constructed())
        {
            return nullptr;
        }
        else
        {
            return reinterpret_cast<base*>(&data[0]);
        }
    }

    base const* get() const
    {
        return reinterpret_cast<base const*>(&data[0]);
    }

    base& operator*()
    {
        return *get();
    }

    base const& operator*() const
    {
        return *get();
    }

    base* operator->()
    {
        return get();
    }

    base const* operator->() const
    {
        return get();
    }

    

    

    template<typename d>
    bool can_downcast() const&
    {
        static_assert((std::is_base_of<base,d>::value || std::is_same<base,d>::value) && sizeof(d) <= cap,"to downcast stack_virt<xb,xc>&& to yb* or stack_virt<yb,xc>, yb must derive from xb and sizeof(yb) must be less than or equal to xc");
        return nullptr != dynamic_cast<d const*>(get());
    }

    template<typename d>
    stack_virt<d,cap> downcast() &&
    {
        static_assert((std::is_base_of<base,d>::value || std::is_same<base,d>::value) && sizeof(d) <= cap,"to downcast stack_virt<xb,xc>&& to y*, yb must derive from xb and");
        assert(can_downcast<d>());
        stack_virt<d,cap> ret;
        ret.set_state_constructed();
        for(int i = 0; i!=data.size(); ++i)
        {
            ret.data[i] = data[i];
        }
        set_state_not_constructed();
        return ret;
    }

    template<typename d>
    d* downcast_get()
    {
        static_assert((std::is_base_of<base,d>::value || std::is_same<base,d>::value) && sizeof(d) <= cap,"to downcast stack_virt<xb,xc>&& to y*, yb must derive from xb and");
        assert(can_downcast<d>());
        return static_cast<d*>(get());
    }

    template<typename d>
    d const* downcast_get() const
    {
        static_assert((std::is_base_of<base,d>::value || std::is_same<base,d>::value) && sizeof(d) <= cap,"to downcast stack_virt<xb,xc>&& to y*, yb must derive from xb and");
        assert(can_downcast<d>());
        return static_cast<d*>(get());
    }

    stack_virt<base,sizeof(base)> shrink() &&
    {
        static_assert(std::is_final<base>::value,"in order to shrink stack_virt<xb,xc> to stack_virt<xb,sizeof(xb)>, xb must not be a final class");
        stack_virt<base,sizeof(base)> ret;
        ret.set_state_constructed();
        for(int i = 0; i!=ret.data.size(); ++i)
        {
            ret.data[i] = data[i];
        }
        set_state_not_constructed();
        return ret;
    }

    //trusts you that the dynamically stored type is not bigger than newcap
    template<size_t newcap> 
    stack_virt<base, newcap> unsafe_set_cap() &&
    {
        stack_virt<base,newcap> ret;
        ret.set_state_constructed();
        for(int i = 0; i!=ret.data.size(); ++i)
        {
            ret.data[i] = data[i];
        }
        set_state_not_constructed();
        return ret;
    }

    ~stack_virt()
    {
        if(check_state_whether_constructed())
        {
            get()->~base();
        }
    }

private:

    void set_state_constructed()
    {
        is_constructed = true;
    }

    void set_state_not_constructed()
    {
        is_constructed = false;
    }

    bool check_state_whether_constructed()
    {
        return is_constructed;
    }


    stack_virt()
    {
        static_assert(cap >= sizeof(base), "cap must be larger than the size of the base class");
        set_state_not_constructed();
    }

    std::array<unsigned char,cap> data;
    bool is_constructed;
};
