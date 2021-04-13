Tool to extract chapters from mpls file.

Usage:

To extract all chapters from SHOW_DISC_01/BDMV/PLAYLIST into disc1_*.txt

    mpls_dump -p disc1 -e SHOW_DISC_01

To extract all chapters from 00001.mpls into disc1_list1_*.txt

    mpls_dump -p disc1_list1 -e 00001.mpls

To extract all chapters from 00001.mpls into disc1_list1_*.txt and limit each segment to just over 25 minutes

    mpls_dump -p disc1_list1 -c 1499 00001.mpls
    # After 1499 seconds, a new chapter file is created for next segment

New parameters:

* -e: split chapters at new m2ts file

* -c <seconds>: split chapters after <seconds> point
