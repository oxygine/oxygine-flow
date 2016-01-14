# oxygine-flow
oxygine-flow is an expansion for oxygine which provides a neat system for dialogue/scene organization based on an asynchronous model of events.

manual:
https://bitbucket.org/oxygine/oxygine-framework/wiki/oxygine-flow


##Example
see https://github.com/oxygine/oxygine-flow/blob/master/examples/HelloFlow/src/example.cpp

```cpp
flow::show(new MyScene, [](Event * event){
        log::messageln("scene closed");

        //show dialog
        flow::show(new MyDialog, [ = ](Event*){
                log::messageln("dialog closed");
            });
    });
```	