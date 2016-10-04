#pragma once
#include "EventDispatcher.h"
#include "Actor.h"
#include "Stage.h"
#include "blocking.h"
#include <functional>

namespace oxygine
{
    class Action
    {
    public:
        std::string id;
    };

    namespace flow
    {
        class SceneEvent : public Event
        {
        public:
            SceneEvent(): Event(0) {}
            std::string action;
        };

        class Flow;
        typedef std::function< void(SceneEvent*) > resultCallback;

        DECLARE_SMART(Scene, spScene);
        DECLARE_SMART(Transition, spTransition);


        class Scene: public EventDispatcher
        {
        public:

            enum
            {
                //fired before entering to scene
                EVENT_ENTERING =        makefourcc('S', 'E', 'n', 't'),

                //fired after leaving scene
                EVENT_LEAVING =         makefourcc('S', 'L', 'e', 'a'),

                //current scene will be shown now with transition
                EVENT_PRE_SHOWING =     makefourcc('S', 'P', 'r', 'S'),

                //current scene was shown, transition is done
                EVENT_POST_SHOWING =    makefourcc('S', 'P', 'o', 'S'),

                //current scene will be hidden with transition
                EVENT_PRE_HIDING =      makefourcc('S', 'P', 'r', 'H'),

                //current scene was hidden with transition
                EVENT_POST_HIDING =     makefourcc('S', 'P', 'o', 'H'),

                //fired when other scene was shown on top of current,
                EVENT_SCENE_SHOWN =     makefourcc('S', 'S', 'S', 'h'),

                //fired when other scene was hidden on top of current and current become active
                EVENT_SCENE_HIDDEN =    makefourcc('S', 'S', 'H', 'd'),

                //back button was pressed (android)
                EVENT_BACK =            makefourcc('S', 'B', 'a', 'c'),

                //fired from destructor
                EVENT_DESTROY =         makefourcc('S', 'D', 'e', 's'),

                EVENT_PRESHOWING = EVENT_PRE_SHOWING,
                EVENT_POSTSHOWING = EVENT_POST_SHOWING,
                EVENT_PREHIDING = EVENT_PRE_HIDING,
                EVENT_POSTHIDING = EVENT_POST_HIDING,
            };

            Scene();
            ~Scene();

            /*closes current scene**/
            void finish(Event* ev = 0);

            /**
            Automatically removes Scene from stack (flow).
            Can't be used for dialogs.
            resultCallback won't be called for such scene.
            */
            void remove();

            void finishOnClick(spActor);

            /**
            adds TouchEvent::CLICK event listener to actor and calls callback
            if "finish" is true scene will be finished.
            */
            void addClickHandler(spActor, const EventCallback& cb, bool finish = false);

            /**
            adds Back (Android) event listener calls callback
            if "finish" is true scene will be finished.
            */
            void addBackHandler(const EventCallback& cb, bool finish = false);

            /**returns finish method as callback for simplified usage from client code*/
            EventCallback   getFinish();

            void setTransitionIn(spTransition t);
            void setTransitionOut(spTransition t);
            void removeTransitions() { _transitionIn = _transitionOut = 0; }

            spActor         getHolder() const { return _holder; }

        protected:

            friend class Flow;
            friend class Transition;

            virtual void _finish(Event*);
            virtual void update() {}

            /**dialog mode*/
            bool _dialog;



            spTransition runTransition(Flow*, spScene current, bool back);
            virtual spTransition _runTransitionIn(Flow*, spScene current);
            virtual spTransition _runTransitionOut(Flow*, spScene current);

            spActor _holder;
            bool _transitionDone;

        private:

            spTransition _transitionIn;
            spTransition _transitionOut;

            bool _remove;
            bool _done;
            bool _inloop;

            void preShowing();
            void postShowing();
            void preHiding();
            void postHiding();
            void entering();
            void leaving();

            void sceneShown(spScene);
            void sceneHidden(spScene);

            SceneEvent _finishEvent;
            resultCallback _resultCB;

        private:

        };
    }

    typedef flow::Scene Frame;
}
