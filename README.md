# bplus-tree-index
A simple in-memory B+Tree implementation with 1)Page Directory, 2)Variable Length Page Formats and 3)Unclustered B+Tree Index.

## Run with the test data
Run the code with the test data provided by TA (in `data/ProjectB_data`)
```bash
make run 
```
The result is redirect to `output.txt`

## Simply build the project
```bash
make
```
Then you can simply run the command line by `./bp_project`, 
or you can redirect the stdin and stdout by `./bp_project < YOUR_TEST_DATA.txt` or `./bp_project < YOUR_TEST_DATA.txt > OUTPUT_DATA.txt`

## Unit-Test
- Test the page size
```bash
make page_size_test
```
If you encounter the error with: `fatal error: bits/XXXdefs.h: No such file or directory`, please install `gcc-multilib` with the following command:
```bash
sudo apt-get install gcc-multilib
```
Then re-run the `make page_size_test`, you will see the size of all kind of pages.

-	Test Slotted Page with Insert/Query
```bash
make slotted_page_test
```

-	Test B+Tree Insert/Query
```bash
make bp_insert_test
```

-	Test B+Tree Delete
```bash
make bp_delete_test
```

-	Test Relation Manipulation
```bash
make relation_test
```

## Clean the directory
```bash
make clean
```
