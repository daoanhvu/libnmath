#!/bin/bash
set -e
set -x

TARGET=$1

CURRENTDIR=`pwd`

if [ ${TARGET} = "Android" ] 
then
	NDK=/Users/vdao/Library/Android/sdk/ndk-bundle
	MINSDKVERSION=23
	ABIES=(armeabi-v7a arm64-v8a x86 x86_64)
	for arch in ${ABIES[@]}; do
		OUTPUTDIR=${CURRENTDIR}/output/Android/${arch}
		echo $OUTPUTDIR
		cmake -H. -B${OUTPUTDIR} -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
	    -DANDROID_ABI=${arch} \
	    -DANDROID_NATIVE_API_LEVEL=$MINSDKVERSION \
	    -DLIBTYPE=SHARED -DTARGET=Android $OTHER_ARGS
		cd ${OUTPUTDIR}
		make clean && make
		cd ${CURRENTDIR}
	done
	
elif [ ${TARGET} = "Linux" ]
then
	OUTPUTDIR=`pwd`/output/Linux
	cmake -H. -B${OUTPUTDIR} -DADDTESTS:STRING="" -DLIBTYPE=STATIC
	cd ${OUTPUTDIR}
	make clean && make
elif [ ${TARGET} = "macOS" ] 
then
	#statements
	OUTPUTDIR=`pwd`/output/macOS
	cmake -H. -B${OUTPUTDIR} -DADDTESTS:STRING="" -DLIBTYPE=STATIC
	cd ${OUTPUTDIR}
	make clean && make
	#run test
	#${OUTPUTDIR}/testParser
	${OUTPUTDIR}/testFunction
else
	# Windows properly
	echo "Invalid target!"
fi
