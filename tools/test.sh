npm install jsni
cd ..
for x in `ls -d */`;do
  if [ "$x" = "tools/" ]; then
    continue
  fi
  cd $x
  ln -s ../tools/node_modules  ./

  node-gyp rebuild 1> t.log 2>~/tmp/$i.log
  node --expose-gc native.js
  if [ $? -ne 0 ]; then
    echo "============Case: $i FAILED!============"
    #cat ~/tmp/$i.log
    rm -f t.log
    echo "Fail: $x\n"
    exit
  else
    echo "Success: $x\n"
  fi
  rm -f t.log
  rm -rf node_modules
  cd ..
done
cd tools
rm -rf node_modules
rm -rf package-lock.json
