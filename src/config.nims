
--app:gui
--mm:orc
--threads:on
--cincludes:"../vendored/include"

when defined(windows):
    --cc:vcc
elif defined(osx):
    --cc:clang
else:
    --cc:gcc
