#!/usr/bin/env bash

set -u 

function error {
echo "ERROR: $1"
exit 1
}

warning_count=0
function warning {
let "warning_count++"
echo "MERGE_WARNING: $1"
}

function start_bar {
count_max="${1:?"provide count max"}"
}

function update_bar {
local count="${1:?"provide count"}"
local message="${2:?"provide message"}"
local percent=$(echo "100*${count}/${count_max}" | bc)
#printf "[%03d/%03d] %3d%% %-80.80s\r" ${count} ${count_max} ${percent} "${message}"
printf "[%03d/%03d] %3d%% %-80.80s\n" ${count} ${count_max} ${percent} "${message}"
}

function end_bar {
unset count_max
#echo -ne '\n'
}

rootMerger="clitkMergeRootFiles"
test -x "./clitkMergeRootFiles" && rootMerger="./clitkMergeRootFiles"

function merge_root {
local merged="$1"
shift
echo "  ${indent}entering root merger"
echo "  ${indent}merger is ${rootMerger}"
echo "  ${indent}creating ${merged}"
#echo "######## $#"
#echo "######## $*"

if test $# -eq 1
then
    echo "  ${indent}just one partial file => just copy it"
    cp "$1" "${merged}"
    return
fi

local count=0
local arguments=" -o ${merged}"
while test $# -gt 0
do
    local partial="$1"
    shift
    let count++
    local arguments=" -i ${partial} ${arguments}"
done
${rootMerger} ${arguments} > /dev/null || warning "error while calling ${rootMerger}"
echo "  ${indent}merged ${count} files"
}

statMerger="mergeStatFile.py"
test -x "./mergeStatFile.sh" && statMerger="./mergeStatFile.sh"

function merge_stat {
local merged="$1"
shift
echo "  ${indent}entering stat merger"
echo "  ${indent}merger is ${statMerger}"
echo "  ${indent}creating ${merged}"
local count=0
start_bar $#
while test $# -gt 0
do
    local partial="$1"
    shift
    let count++

    if test ! -f "${merged}"
    then
        update_bar ${count} "copying first partial result ${partial}"
        cp "${partial}" "${merged}"
        continue
    fi

    update_bar ${count} "adding ${partial}"
    ${statMerger} -i "${merged}" -j "${partial}" -o "${merged}" 2> /dev/null > /dev/null || warning "error while calling ${statMerger}"
done
end_bar
echo "  ${indent}merged ${count} files"
}

txtImageMerger="clitkMergeAsciiDoseActor"
test -f "./clitkMergeAsciiDoseActor" && txtImageMerger="./clitkMergeAsciiDoseActor"

function merge_txt_image {
local merged="$1"
shift
echo "  ${indent}entering text image merger"
echo "  ${indent}merger is ${txtImageMerger}"
echo "  ${indent}creating ${merged}"
local count=0
start_bar $#
while test $# -gt 0
do
    local partial="$1"
    shift
    let count++

    if test ! -f "${merged}"
    then
        update_bar ${count} "copying first partial result ${partial}"
        cp "${partial}" "${merged}"
        continue
    fi

    update_bar ${count} "adding ${partial}"
    local header="$(cat "${merged}" | head -n 6)"
    local tmp="$(mktemp)"
    ${txtImageMerger} -i "${partial}" -j "${merged}" -o "${tmp}" 2> /dev/null > /dev/null || warning "error while calling ${txtImageMerger}"
    echo "${header}" > "${merged}"
    grep -v '## Merge' "${tmp}" >> "${merged}"
    rm "${tmp}"
done
end_bar
echo "  ${indent}merged ${count} files"
}

hdrImageMerger="clitkImageArithm"
test -x "./clitkImageArithm" && hdrImageMerger="./clitkImageArithm"

function merge_hdr_image {
local merged="$1"
local merged_bin="${merged%.*}.img"
shift
echo "  ${indent}entering hdr image merger"
echo "  ${indent}merger is ${hdrImageMerger}"
echo "  ${indent}creating ${merged}"
local count=0
start_bar $#
while test $# -gt 0
do
    local partial="$1"
    local partial_bin="${partial%.*}.img"
    shift
    let count++

    if test ! -f "${merged}"
    then
        update_bar ${count} "copying first partial result ${partial}"
        cp "${partial}" "${merged}"
        cp "${partial_bin}" "${merged_bin}"
        continue
    fi

    update_bar ${count} "adding ${partial}"
    ${hdrImageMerger} -t 0 -i "${partial}" -j "${merged}" -o "${merged}" 2> /dev/null > /dev/null || warning "error while calling ${hdrImageMerger}"
done
end_bar
echo "  ${indent}merged ${count} files"
}

mhdImageMerger="clitkImageArithm"
test -x "./clitkImageArithm" && mhdImageMerger="./clitkImageArithm"

function merge_mhd_image {
local merged="$1"
local merged_bin="${merged%.*}.img"
shift
echo "  ${indent}entering mhd image merger"
echo "  ${indent}merger is ${mhdImageMerger}"
echo "  ${indent}creating ${merged}"
local count=0
start_bar $#
while test $# -gt 0
do
    local partial="$1"
    local partial_bin="${partial%.*}.raw"
    shift
    let count++

    if test ! -f "${merged}"
    then
        update_bar ${count} "copying first partial result ${partial}"
        cp "${partial}" "${merged}"
        cp "${partial_bin}" "${merged_bin}"
        continue
    fi

    update_bar ${count} "adding ${partial}"
    ${mhdImageMerger} -t 0 -i "${partial}" -j "${merged}" -o "${merged}" 2> /dev/null > /dev/null || warning "error while calling ${mhdImageMerger}"
done
end_bar
echo "  ${indent}merged ${count} files"
}

function merge_dispatcher {
    local indent="  ** "
    local outputfile="${1:?"provide output filename"}"
    echo "merging ${outputfile}"

    local partialoutputfiles="$(find "${rundir}" -type f -name "${outputfile}")"
    local nboutputfiles="$(echo "${partialoutputfiles}" | wc -l)"
    if test ${nboutputdirs} -ne ${nboutputfiles}
    then
        warning "missing files"
        return
    fi

    local firstpartialoutputfile="$(echo "${partialoutputfiles}" | head -n 1)"
    local firstpartialoutputextension="${firstpartialoutputfile##*.}"
    echo "${indent}testing file type on ${firstpartialoutputfile}"

    if test "${firstpartialoutputextension}" == "hdr"
    then
        echo "${indent}this is a analyse image"
        local mergedfile="${outputdir}/$(basename "${firstpartialoutputfile}")"
        merge_hdr_image "${mergedfile}" ${partialoutputfiles} || error "error while merging"
        return
    fi

    if test "${firstpartialoutputextension}" == "mhd"
    then
        echo "${indent}this is a mhd image"
        local mergedfile="${outputdir}/$(basename "${firstpartialoutputfile}")"
        merge_mhd_image "${mergedfile}" ${partialoutputfiles} || error "error while merging"
        return
    fi

    if test "${firstpartialoutputextension}" == "root"
    then
        echo "${indent}this is a root file"
        local mergedfile="${outputdir}/$(basename "${firstpartialoutputfile}")"
        merge_root "${mergedfile}" ${partialoutputfiles} || error "error while merging"
        return
    fi

    if test "${firstpartialoutputextension}" == "txt" && grep 'NumberOfEvent' "${firstpartialoutputfile}" > /dev/null
    then
        echo "${indent}this is a stat file"
        local mergedfile="${outputdir}/$(basename "${firstpartialoutputfile}")"
        merge_stat "${mergedfile}" ${partialoutputfiles} || error "error while merging"
        return
    fi

    if test "${firstpartialoutputextension}" == "txt" && grep 'Resol' "${firstpartialoutputfile}" > /dev/null
    then
        local resol="$(sed -nr '/Resol/s/^.*=\s+\((.+)\)\s*$/\1/p' "${firstpartialoutputfile}")"
        local resolx="$(echo "${resol}" | cut -d',' -f1)"
        local resoly="$(echo "${resol}" | cut -d',' -f2)"
        local resolz="$(echo "${resol}" | cut -d',' -f3)"
        if test "${resol}" == "1,1,1"
        then
            echo "${indent}this is a txt integral value"
            local mergedfile="${outputdir}/$(basename "${firstpartialoutputfile}")"
            merge_txt_image "${mergedfile}" ${partialoutputfiles} || error "error while merging"
            return
        fi
        if test \( "${resolx}" == "1" -a "${resoly}" == "1" \) -o \( "${resoly}" == "1" -a "${resolz}" == "1" \) -o \( "${resolz}" == "1" -a "${resolx}" == "1" \)
        then
            echo "${indent}this is a txt profile"
            local mergedfile="${outputdir}/$(basename "${firstpartialoutputfile}")"
            merge_txt_image "${mergedfile}" ${partialoutputfiles} || error "error while merging"
            return
        fi
    fi

    if test "${firstpartialoutputextension}" == "txt"
    then
        echo "${indent}this is a non specific txt output"
        local mergedfile="${outputdir}/$(basename "${firstpartialoutputfile}")"
        echo "  ${indent}catting ${mergedfile}"
        cat ${partialoutputfiles} > "${mergedfile}" || error "error while merging"
        return
    fi

    warning "unknown file type"
}

echo "!!!! this is $0 v0.3h !!!!"

rundir="${1?"provide run dir"}"
rundir="$(echo "${rundir}" | sed 's|/*$||')"
nboutputdirs="$(find "${rundir}" -mindepth 1 -type d -name 'output*' | wc -l)"

test ${nboutputdirs} -gt 0 || error "no output dir found"
echo "found ${nboutputdirs} partial output dirs"

outputdir="results"
if [ "${rundir}" != "." -a "${rundir##*.}" != "${rundir}" ]
then
    outputdir="results.${rundir##*.}"
fi
outputdir="$(basename "${outputdir}")"
echo "output dir is ${outputdir}"
test -d "${outputdir}" && rm -r "${outputdir}"
mkdir "${outputdir}"

for outputfile in $(find "${rundir}" -regextype 'posix-extended' -type f -regex "${rundir}/output.*\.(hdr|root|txt)" | awk -F '/' '{ print $NF; }' | sort | uniq)
do
    merge_dispatcher "${outputfile}"
done

if [ -f "${rundir}/params.txt" ]
then
	echo "copying params file"
	cp "${rundir}/params.txt" "${outputdir}/params.txt"
fi

echo "these was ${warning_count} warning(s)"
