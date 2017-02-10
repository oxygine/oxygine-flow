#include "flow.h"
#include "Scene.h"
#include "Transition.h"
#include "DebugActor.h"
#include "core/oxygine.h"

namespace oxygine
{
    bool checkQuit();

    namespace flow
    {
        int BLOCK_TOUCH_DURATION = 300;

        spActor _touchBlocker;

        Vector2 _blockedTouchPosition(0, 0);
        bool _wasTouchBlocked;
        bool _wasBackBlocked;
        timeMS _tm = 0;


        Flow Flow::instance;


        spTransition _defaultTransition;


        void init()
        {
            _tm = 0;
            _wasTouchBlocked = false;
            _wasBackBlocked = false;
            _touchBlocker = new Actor;
            _touchBlocker->setName("Scene::_touchBlocker");
            _touchBlocker->setPosition(-10000, -10000);
            _touchBlocker->setSize(20000, 20000);
            _touchBlocker->setPriority(10000);
            _touchBlocker->setName("touchBlocker");

            _defaultTransition = new TransitionFade;

            Flow::instance = Flow();
            Flow::instance.init();
        }

        void free()
        {
            _touchBlocker->detach();
            _touchBlocker = 0;
            _defaultTransition = 0;
            Flow::instance.free();
        }


        Flow::Flow()
        {
            _transition = false;
            _transitionDone = false;
            _back = false;
        }

        Flow::~Flow()
        {

        }

        void Flow::init()
        {
        }

        void Flow::free()
        {
            _trans = 0;
            _current = 0;
            _next = 0;
            scenes.clear();
            scenes2show.clear();
        }

        void Flow::show(spScene scene, const resultCallback& cb)
        {
            if (scenes.empty())
            {
                scenes.push_back(scene);
                scene->preEntering();
                scene->preShowing();
                scene->_holder->attachTo(getStage());
                scene->postEntering();
                scene->postShowing();
                scene->_resultCB = cb;
                return;
            }

            auto p = std::find(scenes.begin(), scenes.end(), scene);
            if (p != scenes.end())
            {
                log::error("you can't show scene '%s', it is already in the list", scene->getName().c_str());
                OX_ASSERT(p == scenes.end());
                return;
            }

            scene->_resultCB = cb;
            scenes2show.push_back(scene);

            checkShow();
        }

        void Flow::blockedTouch(Event* ev)
        {
            if (_tm + BLOCK_TOUCH_DURATION > getTimeMS())
                return;
            TouchEvent* event = safeCast<TouchEvent*>(ev);
            _blockedTouchPosition = event->getPointer()->getPosition();
            _wasTouchBlocked = true;
        }

        bool Flow::hasSceneInStack(spScene scene) const
        {
            return std::find(scenes.begin(), scenes.end(), scene) != scenes.end();
        }

        void Flow::phaseBegin(spScene current, spScene next, bool back)
        {
            _back = back;
            _current = current;
            _next = next;

            _transition = true;


            if (next && !_back)
                next->preEntering();

            if (!_back || !current->_dialog)
                next->preShowing();

            if ((next->_dialog && _back) || !next->_dialog)
            {
                if (_current->_done)
                    current->preLeaving();
                current->preHiding();
            }

            if (!back)
                current->sceneShown(next);

            _trans = next->runTransition(this, current, back);

            getStage()->addChild(_touchBlocker);
            _wasTouchBlocked = false;
            _wasBackBlocked = false;
            _tm = getTimeMS();
            getStage()->addEventListener(TouchEvent::CLICK, CLOSURE(this, &Flow::blockedTouch));
        }

        void Flow::phaseEnd()
        {
            _transition = false;

            spScene current = _current;
            spScene next = _next;

            _current = 0;
            _next = 0;

            if ((next->_dialog && _back) || !next->_dialog)
            {
                current->_holder->detach();
                current->postHiding();
            }

            next->getHolder()->insertSiblingBefore(_touchBlocker);

            if (!_back || !current->_dialog)
                next->postShowing();

            if (next && !_back)
                next->postEntering();



            getStage()->removeEventListener(TouchEvent::CLICK, CLOSURE(this, &Flow::blockedTouch));

            if (current->_done)
            {
                current->postLeaving();
                if (current->_resultCB)
                {
                    current->_resultCB(&current->_finishEvent);
                    current->_resultCB = resultCallback();
                    current->_finishEvent = FlowEvent();
                }
            }

            if (_back)
                next->sceneHidden(current);


            if (current->_remove)
            {
                OX_ASSERT(next->_dialog == false);
                std::vector<spScene>::iterator i = std::find(scenes.begin(), scenes.end(), current);
                OX_ASSERT(i != scenes.end());
                scenes.erase(i);
            }


            if (_wasTouchBlocked)
            {
                if (scenes2show.empty())
                {
                    log::messageln("send  blocked touch");
                    TouchEvent click(TouchEvent::CLICK, true, _blockedTouchPosition);
                    getStage()->handleEvent(&click);
                }
                _wasTouchBlocked = false;
            }
        }

        void Flow::checkDone()
        {
            spScene current = scenes.back();
            if (current->_done)
            {
                scenes.pop_back();
                if (scenes.empty())
                {
                    current->_resultCB = resultCallback();
                    current->_finishEvent = FlowEvent();
                }
                else
                {
                    spScene prev = scenes.back();
                    phaseBegin(current, prev, true);
                }
            }
        }

        void Flow::checkShow()
        {
            if (scenes2show.empty())
                return;

            if (scenes.back()->_done)
                return;

            if (_transition)
                return;

            spScene current = scenes.back();


            std::vector<spScene>::iterator it = scenes2show.begin();

            if (!current->_allowDialogsOnTop)
            {
                for (; it != scenes2show.end(); ++it)
                {
                    if ((*it)->_dialog)
                        continue;
                    break;
                }
            }

            if (it == scenes2show.end())
                return;

            spScene next = *it;


            if (current->_dialog)
            {
                OX_ASSERT(next->_dialog && "you can't show fullscreen scene on top of dialog");
                OX_ASSERT(!current->_remove && "you can't remove dialog from flow");
                if (!next->_dialog)
                    return;
            }

            scenes2show.erase(it);

            scenes.push_back(next);
            phaseBegin(current, next, false);

        }

        void Flow::update()
        {
            if (scenes.empty() && scenes2show.empty())
                return;

            if (DebugActor::instance)
            {
                std::string str;
                for (size_t i = 0; i < scenes.size(); ++i)
                {
                    str += scenes[i]->getName();
                    str += "-> ";
                }
                if (!str.empty())
                {
                    str.pop_back();
                    str.pop_back();
                    str.pop_back();
                    DebugActor::addDebugString(str.c_str());
                }
            }

            bool quit = checkQuit();
            static bool quitLast = false;
            if (quit && !quitLast)
            {
                _wasBackBlocked = true;
            }
            quitLast = quit;

            if (_transition)
            {
                //we don't know who controls transition, next, current, or Transition
                //if (_trans && _trans->_done || _next->_transitionDone || _current->_transitionDone)
                if (_transitionDone)
                    phaseEnd();

                return;
            }

            checkDone();
            checkShow();

            if (_transition)
                return;

            if (!scenes.empty())
            {
                spScene current = scenes.back();

                if (_wasBackBlocked)
                {
                    _wasBackBlocked = false;

                    Event ev(Scene::EVENT_BACK);
                    current->dispatchEvent(&ev);
                    checkDone();
                }
                else
                {
                    current->update();
                }
            }

            if (scenes.empty())
                core::requestQuit();
        }

        void update()
        {
            Flow::instance.update();
        }

        void show(spScene s, const resultCallback& cb, Flow* f)
        {
            if (!f)
                f = &Flow::instance;

            f->show(s, cb);
        }

        Flow& get()
        {
            return Flow::instance;
        }
    }
}
