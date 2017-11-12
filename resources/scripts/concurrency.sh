#!/bin/bash

root_path="../httpfs_root/Star Wars"
url_path="Star%20Wars"
reader_file="Obi-Wan Kenobi and Qui-Gon Jinn vs Darth Maul - Blu Ray HD.mp4"
writer_file="'Rogue One' Spliced with 'A New Hope' v.2-209263699.mp4"
common_name="foo.mp4"
port=8080
httpc="../../httpc"

echo "Renaming ${reader_file} to ${common_name}"
cp -a "${root_path}/${reader_file}" "${root_path}/${common_name}"

echo "Chaning ${common_name} through HTTP POST..."
${httpc} post -h "Content-Type: video/mp4" -f "${root_path}/${writer_file}" "http://localhost:${port}/${url_path}/${common_name}"
