#include "Scene.h"
#include "Transition.h"
#include "Clock.h"

#include "flow.h"

#ifdef __S3E__
#include "s3eKeyboard.h"
#include "s3eDevice.h"
#else
#include "SDL.h"
#endif

#define LOGD(...) log::messageln(__VA_ARGS__)
//#define LOGD(...) {}
namespace oxygine
{
    bool checkQuit()
    {
        bool back = false;
#ifdef __S3E__
        if (s3eDeviceCheckQuitRequest() ||
                (s3eKeyboardGetState(s3eKeyBackspace) & S3E_KEY_STATE_DOWN) ||
                (s3eKeyboardGetState(s3eKeyAbsBSK) & S3E_KEY_STATE_DOWN))
        {
            back = true;
        }
#elif EMSCRIPTEN

#else
        const Uint8* data = SDL_GetKeyboardState(0);

        if (data[SDL_GetScancodeFromKey(SDLK_BACKSPACE)] ||
                data[SDL_GetScancodeFromKey(SDLK_ESCAPE)] ||
                data[SDL_GetScancodeFromKey(SDLK_AC_BACK)])
        {
            back = true;
        }

#endif
        return back;
    }

    namespace flow
    {
        extern bool _wasTouchBlocked;
        extern spTransition _defaultTransition;

        Scene::Scene(): _done(false), _remove(false), _dialog(false), _inloop(false), _allowDialogsOnTop(true)
        {
            setName("Scene");
            _holder = new Actor;
            //if (Stage::instance)
            _holder->setSize(Stage::instance->getSize());
            _holder->setName("Scene::_holder");

            _transitionIn = _defaultTransition;
            _transitionOut = _defaultTransition;
        }

        Scene::~Scene()
        {
            //workaround allows to convert this to sp from destructor
            _ref_counter = 10000;
            Event ev(EVENT_DESTROY);
            EventDispatcher::dispatchEvent(&ev);
        }

        void Scene::setTransitionIn(spTransition t)
        {
            _transitionIn = t;
        }

        void Scene::setTransitionOut(spTransition t)
        {
            _transitionOut = t;
        }

        void Scene::setTransition(spTransition tin, spTransition tout)
        {
            setTransitionIn(tin);
            setTransitionOut(tout);
        }

        void Scene::finishOnClick(spActor actor)
        {
            actor->addClickListener(getFinish());
        }

        void Scene::addClickHandler(spActor actor, const EventCallback& cb, bool finish)
        {
            actor->addClickListener(cb);
            if (finish)
                finishOnClick(actor);
        }

        void Scene::addBackHandler(const EventCallback& cb, bool finish)
        {
            addEventListener(EVENT_BACK, cb);
            if (finish)
                addEventListener(EVENT_BACK, getFinish());
        }

        EventCallback   Scene::getFinish()
        {
            return CLOSURE(this, &Scene::finish);
        }

        spTransition Scene::runTransition(Flow* f, spScene current, bool back)
        {
            return back ? _runTransitionOut(f, current) : _runTransitionIn(f, current);
        }

        spTransition Scene::_runTransitionIn(Flow* f, spScene current)
        {
            spTransition t = _transitionIn;
            if (!t)
                return 0;
            t->run(f, current, this, false);
            return t;
        }

        spTransition Scene::_runTransitionOut(Flow* f, spScene current)
        {
            spTransition t = current->_transitionOut;
            if (!t)
                return 0;
            t->run(f, current, this, true);
            return t;
        }

        void Scene::entering()
        {
            LOGD("%-20s '%s'", "Scene.entering", getName().c_str());
            Event ev(EVENT_ENTERING);
            dispatchEvent(&ev);
        }

        void Scene::leaving()
        {
            LOGD("%-20s '%s'", "Scene.leaving", getName().c_str());
            Event ev(EVENT_LEAVING);
            dispatchEvent(&ev);
        }

        void Scene::preLeaving()
        {
            //LOGD("%-20s '%s'", "Scene.preLeaving", getName().c_str());
            Event ev(EVENT_PRE_LEAVING);
            dispatchEvent(&ev);
        }

        void Scene::postLeaving()
        {
            //LOGD("%-20s '%s'", "Scene.postLeaving", getName().c_str());
            Event ev(EVENT_POST_LEAVING);
            dispatchEvent(&ev);
        }

        void Scene::sceneShown(spScene s)
        {
            LOGD("%-20s '%s' - '%s'", "Scene.sceneShown on ", getName().c_str(), s->getName().c_str());
            Event ev(EVENT_SCENE_SHOWN);
            dispatchEvent(&ev);
        }

        void Scene::sceneHidden(spScene s)
        {
            LOGD("%-20s '%s' - '%s'", "Scene.sceneHidden on ", getName().c_str(), s->getName().c_str());
            Event ev(EVENT_SCENE_HIDDEN);
            dispatchEvent(&ev);
        }

        void Scene::preShowing()
        {
            _done = false;
            _remove = false;

            update();

            LOGD("%-20s '%s'", "Scene.preShowing", getName().c_str());
            Event ev(EVENT_PRE_SHOWING);
            dispatchEvent(&ev);
        }

        void Scene::postShowing()
        {
            LOGD("%-20s '%s'", "Scene.postShowing", getName().c_str());
            Event ev(EVENT_POST_SHOWING);
            dispatchEvent(&ev);
        }

        void Scene::preHiding()
        {
            LOGD("%-20s '%s'", "Scene.preHiding", getName().c_str());
            Event ev(EVENT_PRE_HIDING);
            dispatchEvent(&ev);
        }

        void Scene::postHiding()
        {
            LOGD("%-20s '%s'", "Scene.postHiding", getName().c_str());
            //_transitionIn->_current = 0;
            //_transitionIn->_next = 0;
            //_transitionOut->_current = 0;
            //_transitionOut->_next = 0;

            Event ev(EVENT_POST_HIDING);
            dispatchEvent(&ev);
        }

        void Scene::_finish(Event* ev)
        {
            _done = true;
        }

        void Scene::finish(Event* ev)
        {
            _finish(ev);
            if (_done)
            {
                if (ev)
                    static_cast<Event&>(_finishEvent) = *ev;
                else
                    _finishEvent = FlowEvent();
            }

            //Flow::instance.checkDone();
        }

        void Scene::remove()
        {
            _remove = true;
            _resultCB = resultCallback();
            OX_ASSERT(_dialog == false);
        }
    }
}