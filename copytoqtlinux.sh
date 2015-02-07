mkdir -p ../g-ear-qt/lib/linux
cp libcore.a ../g-ear-qt/lib/linux/libcore-qt.a
rm -r ../g-ear-qt/include/core
cp -r core/public ../g-ear-qt/include/core
