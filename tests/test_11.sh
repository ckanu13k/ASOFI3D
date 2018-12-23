#!/usr/bin/env bash
# Regression test 11.
# Check that the seismograms are comparable with the seismograms
# obtained using SAVA code (https://github.com/daniel-koehn/SAVA).
# Model is one-layer orthorhombic.
. tests/functions.sh

MODEL="src/model_elastic.c"
TEST_PATH="tests/fixtures/test_11"
TEST_ID="TEST_11"

# Setup function prepares environment for the test (creates directories).
setup

# Preserve old model.
mv $MODEL ${MODEL}.bak.${TEST_ID}

on_exit() {
    # Clean up function to be called at the end of the test.
    mv ${MODEL}.bak.${TEST_ID} $MODEL
}

# Execute function 'on_exit' when this script exits.
trap on_exit INT TERM EXIT

# Copy input to the directory where the test is executed.
cp "${TEST_PATH}/model_elastic.c"    src/model_elastic.c
cp "${TEST_PATH}/sofi3D.json"        tmp/in_and_out/sofi3D.json
cp "${TEST_PATH}/source.dat"         tmp/sources/
cp "${TEST_PATH}/receiver.dat"       tmp/receiver/

# Compile code.
cd src
make sofi3D > ../tmp/make.log
if [ "$?" -ne "0" ]; then
    cd ..
    echo "${TEST_ID}: FAIL" > /dev/stderr
    exit 1
fi
cd ..

# Run code.
echo "${TEST_ID}: Running solver. Output is captured to tmp/ASOFI3D.log"
./run_ASOFI3D.sh 16 tmp/ > tmp/ASOFI3D.log &
task_id=$!
animate_progress $task_id "${TEST_ID}: Running solver"

wait $task_id
code=$?

if [ "$code" -ne "0" ]; then
    echo "${TEST_ID}: FAIL Running ASOFI3D failed" > /dev/stderr
    exit 1
fi

# Convert seismograms in SEG-Y format to the Madagascar RSF format.
convert_segy_to_rsf tmp/su/test_p.sgy
convert_segy_to_rsf ${TEST_PATH}/su/ref_p.sgy

# Read the files.
# Compare with the recorded output.
tests/compare_datasets.py tmp/su/test_p.rsf ${TEST_PATH}/su/ref_p.rsf \
    --rtol=1e-10 --atol=1e-10
result=$?
if [ "$result" -ne "0" ]; then
    echo "${TEST_ID}: Seismograms (pressure) differ" > /dev/stderr
    exit 1
fi

echo "${TEST_ID}: PASS"