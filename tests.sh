#!/bin/bash

rm -rf test_files
mkdir test_files
cd test_files

PASS=0
FAIL=0

echo -n "Testing WRITE"
echo "Hello from the other side.." > test.txt
../client/rfs WRITE test.txt test.txt > /dev/null 2>&1
if [ -f "../server/data_files/test.txt" ]; then
    echo "ok"
    ((PASS++))
else
    echo "failed"
    ((FAIL++))
fi


echo -n "Testing GET"
../client/rfs GET test.txt download.txt > /dev/null 2>&1
if diff test.txt download.txt > /dev/null 2>&1; then
    echo "ok"
    ((PASS++))
else
    echo "failed"
    ((FAIL++))
fi


echo -n "Testing versioning"
echo "Hello from the other version..." > test.txt
../client/rfs WRITE test.txt test.txt > /dev/null 2>&1
if [ -f "../server/data_files/test.txt.v1" ]; then
    echo "ok"
    ((PASS++))
else
    echo "failed"
    ((FAIL++))
fi


echo -n "Testing GET old version"
../client/rfs GET test.txt.v1 old-test.txt > /dev/null 2>&1
if grep -q "Hello from the other side.." old-test.txt; then
    echo "ok"
    ((PASS++))
else
    echo "failed"
    ((FAIL++))
fi


echo -n "Testing LS"
OUT=$(../client/rfs LS test.txt 2>&1)
if echo "$OUT" | grep -q "v1"; then
    echo "ok"
    ((PASS++))
else
    echo "failed"
    ((FAIL++))
fi


echo -n "Testing RM"
../client/rfs RM test.txt > /dev/null 2>&1
if [ ! -f "../server/data_files/test.txt" ]; then
    echo "ok"
    ((PASS++))
else
    echo "failed"
    ((FAIL++))
fi


echo -n "Testing GET nonexistent file"
OUT=$(../client/rfs GET dummy.txt dummy.txt 2>&1)
if echo "$OUT" | grep -qi "error"; then
    echo "ok"
    ((PASS++))
else
    echo "failed"
    ((FAIL++))
fi


echo -n "Testing subdirectory"
echo "Hello from a subdirectory..." > test-new.txt
../client/rfs WRITE test-new.txt folder/test-new.txt > /dev/null 2>&1
if [ -f "../server/data_files/folder/test-new.txt" ]; then
    echo "ok"
    ((PASS++))
else
    echo "failed"
    ((FAIL++))
fi


echo -n "Testing concurrent clients"
echo "Hello from client1" > c1.txt
echo "Hello from client2" > c2.txt
echo "Hello from client3" > c3.txt

../client/rfs WRITE c1.txt c1.txt > /dev/null 2>&1 &
../client/rfs WRITE c2.txt c2.txt > /dev/null 2>&1 &
../client/rfs WRITE c3.txt c3.txt > /dev/null 2>&1 &

wait

if [ -f "../server/data_files/c1.txt" ] && [ -f "../server/data_files/c2.txt" ] && [ -f "../server/data_files/c3.txt" ]; then
    echo "ok"
    ((PASS++))
else
    echo "failed"
    ((FAIL++))
fi

cd ..
rm -rf test_files

echo "All tests done: $PASS passed, $FAIL failed"