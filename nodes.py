import bisect
import pymongo
import tangelo
import bson.json_util
import json

class MongoCollectionList:
    def __init__(self, c):
        self.c = c

    def __getitem__(self, i):
        return self.c.find_one({"_id": i}, fields=["name"])["name"]

    def __len__(self):
        return self.c.count()

tld = [
    d for d in pymongo.MongoClient()["xdata"]["wdc.tld"]
    .find(sort=[("value", pymongo.DESCENDING)])
]

tld_sum = 0
tld_offset = {}
for i in xrange(len(tld)):
    tld_offset[tld[i]["_id"]] = tld_sum
    tld_sum += tld[i]["value"]

collection = pymongo.MongoClient()["xdata"]["wdc.pld.tldind"]
collection_list = MongoCollectionList(collection)
overflow_collection = pymongo.MongoClient()["xdata"]["wdc.pld.overflow"]


def binary_search(a, x, lo=0, hi=None):
    hi = hi if hi is not None else len(a)
    pos = bisect.bisect_left(a, x, lo, hi)
    return (pos if pos != hi and a[pos] == x else -1)


def generate_mcl_matrix(G):
    idx = {}
    for i in range(len(G)):
        idx[G[i]['_id']] = i

    mcifile = ""
    mclheader = (
        ("\n(mclheader\n"),
        ("mcltype matrix\n"),
        ("dimensions %sx%s\n)\n" % (len(G), len(G))),
        ("(mclmatrix\n"),
        ("begin\n")
    )
    for mhline in mclheader:
        mcifile += mhline
    for v in G:
        mcifile += str(idx[v['_id']]) + " "
        for n in v['out']:
            mcifile += str(idx[n]) + " "
        mcifile += " $\n"
    mcifile += ")\n"
    return mcifile


def run_mcl_cluster(G):
    from subprocess import Popen, PIPE
    mcistr = generate_mcl_matrix(G)
    print "=====mcistr begin"
    print mcistr + "\n"
    print "=====mcistr end"
    cmd = "/code/mcl-12-068-install/bin/mcl - -scheme 6 -I 4.0 -o -"
    p = Popen(cmd, stdout=PIPE, stdin=PIPE, stderr=PIPE, shell=True)
    sout, serr = p.communicate(mcistr)
    p.wait()
    print "return code: %i\n" % p.returncode

    idx = {}
    for i in range(len(G)):
        idx[G[i]['_id']] = i

    if p.returncode == 0 and len(sout) > 0:
        lines = sout.splitlines()
        idx_begin = lines.index("begin") + 1
        idx_end = lines[idx_begin:].index(")") + idx_begin
        rejoin = ' '.join(lines[idx_begin:idx_end])
        resplit = [[int(d) for d in line.split()[1:]] for line in rejoin.split("$")]
        print resplit

        cluster = [0] * len(G)
        for c in range(len(resplit)):
            for v in resplit[c]:
                cluster[v] = c
        print cluster
        return cluster
    raise Exception("mcl failed")


@tangelo.types(id=int, ids=json.loads, domain=json.loads)
@tangelo.return_type(bson.json_util.dumps)
def run(name=None, id=None, ids=None, domain=None, cluster=None):
    if domain:
        domain = set(domain)

    if name:
        doc = collection.find_one(
            {"_id": binary_search(collection_list, name)})
        doc["tldid"] = tld_offset[doc["tld"]] + doc["tldid"]
        if "out" not in doc:
            doc["out"] = []
            doc["in"] = []
            doc["inout"] = []
            docs = overflow_collection.find({"node": doc["_id"]})
            for d in docs:
                if "out" in d:
                    doc["out"] = doc["out"] + d["out"]
                if "inout" in d:
                    doc["inout"] = doc["inout"] + d["inout"]
                if "in" in d:
                    doc["in"] = doc["in"] + d["in"]
        return doc
    if id:
        return collection.find_one(id)
    if ids:
        import scipy
        import scipy.cluster.vq
        import numpy
        import operator

        result = [
            {
                "_id": d["_id"],
                "name": d["name"],
                "tld": d["tld"],
                "tldid": tld_offset[d["tld"]] + d["tldid"],
                "out": (
                    [] if "out" not in d else (
                        list(domain.intersection(d["out"])) if domain else d["out"])
                    )
            }
            for d in collection.find(
                {"_id": {"$in": ids}},
                fields=["out", "name", "tld", "tldid"]
            )
        ]

        idx = {}
        for i in range(len(result)):
            idx[result[i]['_id']] = i

        if cluster == "mcl":
            undirected = [{"_id": d["_id"], "out": [n for n in d["out"]]} for d in result]
            for d in undirected:
                for n in d["out"]:
                    undirected[idx[n]]["out"].append(d["_id"])

            cluster = run_mcl_cluster(undirected)

            pairs = zip(result, cluster)
            sorted_pairs = sorted(pairs, key=operator.itemgetter(1))
            return [
                {
                    "_id": d[0]["_id"],
                    "out": d[0]["out"],
                    "name": d[0]["name"],
                    "tld": d[0]["tld"],
                    "tldid": d[0]["tldid"],
                    "cluster": d[1]
                }
                for d in sorted_pairs
            ]

        if cluster == "kmeans":
            m = numpy.zeros((len(ids), len(ids)))

            for d in result:
                for r in d['out']:
                    # m[idx[d['_id']]][idx[r]] = 1
                    m[idx[d['_id']]][idx[r]] = 1

            centers, _ = scipy.cluster.vq.kmeans(m, 2)
            cluster, _ = scipy.cluster.vq.vq(m, centers)
            pairs = zip(result, cluster)
            sorted_pairs = sorted(pairs, key=operator.itemgetter(1))
            return [
                {
                    "_id": d[0]["_id"],
                    "out": d[0]["out"],
                    "name": d[0]["name"],
                    "tld": d[0]["tld"],
                    "tldid": d[0]["tldid"],
                    "cluster": d[1]
                }
                for d in sorted_pairs
            ]

        return result

    return []
