cmake_minimum_required(VERSION 3.25)
project(DeftRPC)

aux_source_directory(. CLSN_HASH)

add_library(CLSN_HASH SHARED ${CLSN_HASH})



