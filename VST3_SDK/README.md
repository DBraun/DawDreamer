This is a stripped down version of the Steinberg VST3 SDK 3.6.7. See `base/LICENSE.txt` and `public.sdk/LICENSE.txt`
for the respective licenses. The code has been obtained from their public repository with the following commnds:

```
MODULES="base pluginterfaces public.sdk"
git clone https://github.com/steinbergmedia/vst3sdk.git tmp_vst3sdk
cd tmp_vst3sdk
git checkout vstsdk367_03_03_2017_build_352
git submodule init $MODULES
git submodule update $MODULES
for MODULE in $MODULES; do rm $MODULE/.git; done
rm -rf public.sdk/samples
mv $MODULES ..
cd ..
rm -rf tmp_vst3sdk
```
