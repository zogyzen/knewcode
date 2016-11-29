#pragma once

namespace KC
{
    namespace _Private
    {
        // 空类
        struct TVoidClass {};

        // 取类型
        template<unsigned i, typename TList>
        struct TGetType
        {
            typedef typename TGetType<i - 1, typename TList::list_type>::list_type list_type;
            typedef typename TGetType<i - 1, typename TList::list_type>::val_type val_type;
        };

        template<typename TList>
        struct TGetType<0, TList>
        {
            typedef TList list_type;
            typedef typename TList::val_type val_type;
        };

        // 取引用
        template<unsigned i, typename TList>
        struct TGetListRef
        {
            static typename TGetType<i, TList>::list_type& GetList(TList& ls)
            { return TGetListRef<i - 1, typename TList::list_type>::GetList(ls.lst); }
            static const typename TGetType<i, TList>::list_type& GetListConst(const TList& ls)
            { return TGetListRef<i - 1, typename TList::list_type>::GetListConst(ls.lst); }
        };

        template<typename TList>
        struct TGetListRef<0, TList>
        {
            static TList& GetList(TList& ls) { return ls; }
            static const TList& GetListConst(const TList& ls) { return ls; }
        };

        /// 值列表
        template<typename TVal, typename TList>
        struct TValList
        {
            // 当前值
            TVal val = TVal();
            // 设置、得到值
            template<unsigned i> void SetVal(const typename TGetType<i, TValList>::val_type& v)
            { TGetListRef<i, TValList>::GetList(*this).val = v; }
            template<unsigned i> typename TGetType<i, TValList>::val_type& GetVal(void)
            { return TGetListRef<i, TValList>::GetList(*this).val; }
            template<unsigned i> const typename TGetType<i, TValList>::val_type& GetVal(void) const
            { return TGetListRef<i, TValList>::GetListConst(*this).val; }

            // 其它值列表
            TList lst;

            // 构造
            TValList() = default;
            virtual ~TValList(void) = default;
            // 拷贝
            TValList(const TValList& clone) : val(clone.val), lst(clone.lst) {}
            TValList& operator=(const TValList& clone) { this->val = clone.val; this->lst = clone.lst; return *this; }

            // 值、列表类型
            typedef TVal val_type;
            typedef TList list_type;
        };

        template<typename TVal, typename TList>
        struct TValList<TVal, TValList<TVoidClass, TList>>
        {
            // 当前值
            TVal val = TVal();
            // 设置、得到值
            template<unsigned i> void SetVal(const TVal& v) { val = v; }
            template<unsigned i> TVal& GetVal(void) { return val; }
            template<unsigned i> const TVal& GetVal(void) const { return val; }

            // 构造
            TValList() = default;
            virtual ~TValList(void) = default;
            // 拷贝
            TValList(const TValList& clone) : val(clone.val) {}
            TValList& operator=(const TValList& clone) { this->val = clone.val; return *this; }

            // 值类型
            typedef TVal val_type;
        };

        template<typename TVal>
        struct TValList<TVal, TVoidClass>
        {
            // 当前值
            TVal val = TVal();
            // 设置、得到值
            template<unsigned i> void SetVal(const TVal& v) { val = v; }
            template<unsigned i> TVal& GetVal(void) { return val; }
            template<unsigned i> const TVal& GetVal(void) const { return val; }

            // 构造
            TValList() = default;
            virtual ~TValList(void) = default;
            // 拷贝
            TValList(const TValList& clone) : val(clone.val) {}
            TValList& operator=(const TValList& clone) { this->val = clone.val; return *this; }

            // 值类型
            typedef TVal val_type;
        };

        template<typename TList>
        struct TValList<TVoidClass, TValList<TVoidClass, TList>>
        {
            // 构造
            TValList() = default;
            virtual ~TValList(void) = default;
            // 拷贝
            TValList(const TValList& clone) = default;
            TValList& operator=(const TValList& clone) = default;
        };

        template<>
        struct TValList<TVoidClass, TVoidClass>
        {
            // 构造
            TValList() = default;
            virtual ~TValList(void) = default;
            // 拷贝
            TValList(const TValList& clone) = default;
            TValList& operator=(const TValList& clone) = default;
        };
    }

    // 值列表
    template<
                typename TBaseClass, int ValListID,
                typename TVal0 = _Private::TVoidClass, typename TVal1 = _Private::TVoidClass,
                typename TVal2 = _Private::TVoidClass, typename TVal3 = _Private::TVoidClass,
                typename TVal4 = _Private::TVoidClass, typename TVal5 = _Private::TVoidClass,
                typename TVal6 = _Private::TVoidClass, typename TVal7 = _Private::TVoidClass,
                typename TVal8 = _Private::TVoidClass, typename TVal9 = _Private::TVoidClass,
                typename TVal10 = _Private::TVoidClass, typename TVal11 = _Private::TVoidClass
            >
    struct TKcTuple : public TBaseClass
    {
        // 值列表
        typedef _Private::TValList<TVal0, _Private::TValList<TVal1, _Private::TValList<TVal2,
                                          _Private::TValList<TVal3, _Private::TValList<TVal4,
                                          _Private::TValList<TVal5, _Private::TValList<TVal6,
                                          _Private::TValList<TVal7, _Private::TValList<TVal8,
                                          _Private::TValList<TVal9, _Private::TValList<TVal10,
                                          _Private::TValList<TVal11,
                                          _Private::TValList<_Private::TVoidClass, _Private::TVoidClass
                                >>>>>>>>>>>>> TValListType;
        TValListType valList;

        // 获取值类型模板
        template<unsigned i> using TGetType = _Private::TGetType<i, TValListType>;

        // 值操作类型
        unsigned ValOPType = 0;

        // 设置、得到值
        template<unsigned i>
        void SetVal(const typename TGetType<i>::val_type& v)
        { ValOPType = i; valList.SetVal<i>(v); }
        template<unsigned i>
        typename TGetType<i>::val_type& GetVal(void)
        { return valList.GetVal<i>(); }
        template<unsigned i>
        const typename TGetType<i>::val_type& GetVal(void) const
        { return valList.GetVal<i>(); }

        // 通过模版类引出【设置、得到值】函数，应对某些编译器不支持，模板函数的嵌套调用
        template<unsigned i>
        struct TSetGetVal
        {
            TSetGetVal(TKcTuple& tp) : m_tp(tp) {}
            typedef typename TGetType<i>::val_type val_type;
            void Set(const val_type& v) { m_tp.SetVal<i>(v); }
            val_type& Get(void) { return m_tp.GetVal<i>(); }

        private:
            TKcTuple& m_tp;
        };

        // 获取值列表编号
        virtual int GetID() const { return ValListID; }

        // 构造
        TKcTuple() = default;
        virtual ~TKcTuple(void) = default;
        // 拷贝
        TKcTuple(const TKcTuple& clone) : TBaseClass(clone), valList(clone.valList) {}
        TKcTuple& operator=(const TKcTuple& clone)
        { TBaseClass::operator=(clone); this->valList = clone.valList; return *this; }
    };
}
