PyMongo
=======

ETL script from Jeff Baumes to build a mongo database for tangelo webgraph viz.

From Jeff:

>  Here is a more complete solution that should get you all the
>  data. Run arc-to-json.py with the node/arc files in the same
>  directory, and also import the attached wdc.tld.json to get the
>  wdc.tld data - it's tiny.
>
>  mongorestore --db xdata --collection wdc.tld wdc.tld.bson