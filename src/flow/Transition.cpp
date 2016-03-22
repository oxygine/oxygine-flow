#include "Transition.h"
#include "RenderState.h"
#include "MaskedSprite.h"
#include "core/oxygine.h"
#include "flow.h"
#include "STDMaterial.h"

#if OXYGINE_RENDERER>2
#   include "TweenAlphaFade.h"
#endif

namespace oxygine
{

    namespace flow
    {

        Transition::Transition() : _done(false), _singleDirection(false), _flow(0)
        {

        }

        void Transition::run(Flow* f, spScene current, spScene next, bool back)
        {
            _flow = f;
            _current = current;
            _next = next;
            _done = false;

            _attach(current, next, back);
            _run(current, next, back);
        }

        void Transition::waitTween(spTween t)
        {
            t->setDoneCallback([ = ](Event*)
            {
                _done = true;
                _clear();
                _current = 0;
                _next = 0;
                _flow->phaseEnd();
            });
        }

        void Transition::_attach(spScene current, spScene next, bool back)
        {
            if (current->_dialog)
            {
                if (!back)
                    getStage()->insertChildAfter(next->_holder, current->_holder);
            }
            else
            {
                if (back)
                    getStage()->insertChildBefore(next->_holder, current->_holder);
                else
                    getStage()->insertChildAfter(next->_holder, current->_holder);
            }
        }

        void Transition::_run(spScene current, spScene next, bool back)
        {
            _done = true;
        }

        void TransitionMove::assign(Scene* scene)
        {
            spTransition t = new TransitionMove;
            scene->setTransitionIn(t);
            scene->setTransitionOut(t);
        }

        TransitionMove::TransitionMove()
        {
            _fade = new ColorRectSprite;
            _fade->setPosition(-10000, -10000);
            _fade->setSize(Vector2(30000, 30000));
            _fade->setColor(Color(0, 0, 0, 128));
            //_fade->setAlpha(0);
        }

        void TransitionMove::_run(spScene current, spScene next, bool back)
        {
            Vector2 _src = Vector2(0.0f, -(float)getStage()->getHeight() + 0);
            Vector2 _dest = Vector2(0.0f, 0.0f);
            int _duration = 500;


            spScene target = back ? current : next;

            spActor holder = target->getHolder();
            if (back)
            {
                std::swap(_src, _dest);

                spTween t = _fade->addTween(Actor::TweenAlpha(0), _duration);
                t->setDetachActor(true);
            }
            else
            {
                _fade->setAlpha(0);
                _fade->addTween(Actor::TweenAlpha(255), _duration);
                holder->getParent()->insertChildBefore(_fade, holder);
            }


            holder->setPosition(_src);
            spTween tween = holder->addTween(Actor::TweenPosition(_dest), _duration, 1, false, 0, Tween::ease_inOutBack);
            waitTween(tween);
        }

        void TransitionFade::assign(Scene* scene)
        {
            spTransition t = new TransitionFade;
            scene->setTransitionIn(t);
            scene->setTransitionOut(t);
        }

        void TransitionFade::_run(spScene current, spScene next, bool back)
        {
            spScene target = back ? current : next;

            int duration = 500;
#if OXYGINE_RENDERER>3
            //target->getHolder()->setAlpha(back ? 255 : 0);
            spTween tween = target->getHolder()->addTween(
                                TweenAlphaFade(!back, PostProcessOptions().fullscreen()), duration, 1, false, 0);
#elif OXYGINE_RENDERER>2
            //target->getHolder()->setAlpha(back ? 255 : 0);
            spTween tween = target->getHolder()->addTween(
                                TweenAlphaFade(!back, TweenAlphaFade::opt_fullscreen), duration, 1, false, 0);
#else
            target->getHolder()->setAlpha(back ? 255 : 0);
            spTween tween = target->getHolder()->addTween(
                                Actor::TweenAlpha(back ? 0 : 255), duration, 1, false, 0);
#endif
            waitTween(tween);
        }

        TransitionShutters::TransitionShutters()
        {
            left = new ColorRectSprite;
            left->setWidth(getStage()->getWidth() / 2);
            left->setHeight(getStage()->getHeight());

            right = new ColorRectSprite;
            right->setWidth(getStage()->getWidth() / 2);
            right->setHeight(getStage()->getHeight());
        }

        void TransitionShutters::_attach(spScene current, spScene next, bool back)
        {

        }

        void TransitionShutters::_run(spScene current, spScene next, bool back)
        {
            getStage()->insertChildAfter(left, current->getHolder());
            getStage()->insertChildAfter(right, current->getHolder());

            int dur = 500;
            left->setX(-left->getWidth());
            right->setX(getStage()->getWidth());

            left->addTween(Actor::TweenX(0), dur);
            spTween t = right->addTween(Actor::TweenX(getStage()->getWidth() / 2), dur);
            t->setDoneCallback([ = ](Event*)
            {
                getStage()->insertChildAfter(next->getHolder(), current->getHolder());
                current->getHolder()->detach();

                left->addTween(Actor::TweenX(-left->getWidth()), dur);
                spTween a = right->addTween(Actor::TweenX(getStage()->getWidth()), dur);
                waitTween(a);
            });
        }

        void TransitionShutters::_clear()
        {
            left->detach();
            right->detach();
        }


        void TransitionShutters::assign(Scene* scene)
        {
            spTransition t = new TransitionShutters;
            scene->setTransitionIn(t);
            scene->setTransitionOut(t);
        }



        TransitionQuads::TransitionQuads() : _center(0, 0)
        {
            //_singleDirection = true;
            getStage()->addEventListener(TouchEvent::CLICK, CLOSURE(this, &TransitionQuads::clicked));
        }

        TransitionQuads::~TransitionQuads()
        {
            getStage()->removeEventListeners(this);
        }

        void TransitionQuads::assign(Scene* scene)
        {
            spTransition t = new TransitionQuads;
            scene->setTransitionIn(t);
            scene->setTransitionOut(t);
        }

        void TransitionQuads::clicked(Event* ev)
        {
            TouchEvent* te = (TouchEvent*)ev;
            _center = getStage()->local2global(te->localPosition);
        }


        void TransitionQuads::update(const UpdateState& us)
        {
            Color b(0, 0, 0, 0);

            Point ds = core::getDisplaySize();
            Rect vp(Point(0, 0), ds);


            IVideoDriver::instance->setRenderTarget(_mask);

#if OXYGINE_RENDERER>2

            Material::setCurrent(0);

            STDRenderer& r = *STDMaterial::instance->getRenderer();
            RenderState rs;
            rs.material = STDMaterial::instance;
            r.initCoordinateSystem(ds.x, ds.y, true);
            //TweenAlphaFade
            //r.Renderer::begin(0);

            {
                _holder->setPosition(Vector2(0, 0));
                _holder->setVisible(true);
                _holder->render(rs);
                //rs.material->render(_holder.get(), rs);
                _holder->setPosition(getStage()->global2local(Vector2(0, 0)));
                //_holder->setVisible(false);
                //r.end();
                rs.material->finish();
            }

#else

            RenderState rs;
            rs.renderer = &_r;

            _r.begin(0);
            {
                _holder->setPosition(Vector2(0, 0));
                _holder->setVisible(true);
                _holder->render(rs);
                _holder->setPosition(getStage()->global2local(Vector2(0, 0)));
                _holder->setVisible(false);
                _r.end();
            }
#endif

            IVideoDriver::instance->setRenderTarget(0);

        }

        void TransitionQuads::_run(spScene current, spScene next, bool back)
        {
            Point ds = core::getDisplaySize();
            spActor holder = new Actor;
            holder->setPosition(getStage()->global2local(Vector2(0, 0)));
            //holder->setVisible(false);
            holder->setSize(core::getDisplaySize());
            holder->attachTo(getStage());
            holder->setPriority(1000);

            int numX = ds.x / 40;
            int numY = ds.y / 40;

            //log::messageln("tq1");
            Vector2 quad(holder->getWidth() / numX, holder->getHeight() / numY);
            spTween slowestTween;

            for (int y = 0; y < numY; ++y)
            {
                for (int x = 0; x < numX; ++x)
                {
                    spSprite sp = new ColorRectSprite;
                    Vector2 pos(quad.x * x, quad.y * y);
                    pos += quad / 2;
                    sp->setPosition(pos);
                    sp->setAnchor(Vector2(0.5f, 0.5f));
                    sp->setSize(quad);
                    sp->attachTo(holder);
                    sp->setScale(0);
                    sp->setColor(Color(0xffffffff));
                    Vector2 d = pos - _center;
                    float time = d.length() / holder->getSize().length();
                    /*
                    if (back)
                    time = 1.0f - time;
                    */

                    float mp = 4.0f;
                    //mp = 10;
                    int tm = int(1 + time * 800 * mp);
                    spTween nt = sp->addTween(Actor::TweenScale(1.0f), int(100 * mp), 1, false, tm);
                    if (!slowestTween || (int)slowestTween->getDelay() < tm)
                        slowestTween = nt;
                }
            }

            //log::messageln("tq2");


#if OXYGINE_RENDERER <= 2

            STDRenderer r;
            RenderState rs;
            rs.renderer = &r;
            rs.renderer->initCoordinateSystem(ds.x, ds.y, true);

            spNativeTexture mask = IVideoDriver::instance->createTexture();
            mask->init(ds.x, ds.y, TF_R5G5B5A1, true);

            _mask = mask;


            spSprite maskSprite = new Sprite;
            {
                AnimationFrame fr;
                Diffuse df;
                df.base = mask;
                RectF srcRect(0, 0, (float)ds.x / mask->getWidth(), (float)ds.y / mask->getHeight());
                RectF destRect(Vector2(0, 0), ds);
                fr.init(0, df, srcRect, destRect, ds);
                maskSprite->setAnimFrame(fr);
            }

            //log::messageln("tq3");

            //#define BUG
            //STDMaterial
            spMaskedSprite bg = new MaskedSprite;
            bg->setMask(maskSprite);

            bg->attachTo(getStage());
            bg->setPriority(100);


            bg->addChild(next->getHolder());
            bg->setInputEnabled(false);
            _bg = bg;

            timeMS tm = getTimeMS() + 3000;

            holder->setCallbackDoUpdate(CLOSURE(this, &TransitionQuads::update));



            //_r = r;
            _holder = holder;
            waitTween(slowestTween);
#endif
        }

        void TransitionQuads::_clear()
        {
            _next->getHolder()->attachTo(getStage());

            _holder->detach();
            _bg->detach();
            _bg->setMask(0);

            _mask->release();
            _mask = 0;
        }
    }
}