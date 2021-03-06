#!/usr/bin/env bash
# Regression test 04.
# Check that the seismograms are the same if we swap a source and receiver
# while the medium is anisotropic.
. tests/functions.sh

readonly MODEL="src/model_elastic.c"
readonly TEST_PATH="tests/fixtures/test_04"
readonly TEST_ID="TEST_04"

setup

backup_default_model

# Copy test model.
cp "${TEST_PATH}/model_elastic.c"    src/
cp "${TEST_PATH}/asofi3D.json"       tmp/in_and_out/
cp "${TEST_PATH}/source.dat"         tmp/sources/
cp "${TEST_PATH}/receiver.dat"       tmp/receiver/

compile_code

run_solver np=16 dir=tmp log=ASOFI3D.log

# Convert seismograms in SEG-Y format to the Madagascar RSF format.
sfsegyread tape=tmp/su/test_p.sgy.shot1 \
    tfile=tmp/su/test_p_trace.rsf.shot1 \
    > tmp/su/test_p.rsf.shot1
sfsegyread tape=tmp/su/test_p.sgy.shot2 \
    tfile=tmp/su/test_p_trace.rsf.shot2 \
    > tmp/su/test_p.rsf.shot2

# Extract traces.
# For source 1 we extract trace 2, while for source 2 we extract trace 1.
# Madagascar enumerates traces starting with 0, that's why min2 and max2
# have such values.
sfwindow < tmp/su/test_p.rsf.shot1 min2=1 > tmp/su/trace2.rsf
sfwindow < tmp/su/test_p.rsf.shot2 max2=0 > tmp/su/trace1.rsf

# Read the files.
# Compare with the recorded output.
tests/compare_datasets.py \
    tmp/su/trace1.rsf tmp/su/trace2.rsf \
    --rtol=1e-10 --atol=1e-8
result=$?
if [ "$result" -ne "0" ]; then
    error "Traces differ"
fi

log "PASS"
