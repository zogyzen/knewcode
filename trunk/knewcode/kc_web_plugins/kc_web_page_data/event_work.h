#pragma once

namespace KC
{
    // 事件管理器
    class CActionData;
    class CEventWork
    {
    public:
        CEventWork(CActionData&);

        // 添加事件
        void AddEvent(TKcEventDef&);

        // 触发事件
        void TrigerEvent(string, const TKcEventDef::EOpPlace);

    private:
        CActionData& m_act;
        // 事件列表
        typedef vector<TKcEventDef*> TEventVector;
        typedef boost::shared_ptr<TEventVector> TEventVectorPtr;
        typedef map<int, TEventVectorPtr> TEventLevelMap;
        typedef boost::shared_ptr<TEventLevelMap> TEventLevelMapPtr;
        typedef map<string, TEventLevelMapPtr> TEventNameMap;
        TEventNameMap m_EventMap;
    };

    // 完成页处理事件
    constexpr const char* c_Event_RenderPage = "FinishRenderPage";
}
