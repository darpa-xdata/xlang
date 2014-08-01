import tangelo
import bson.json_util
import json
import requests

def add_extras(ret):
    ret["tld"] = ".edu"
    ret["tldid"] = ret["id"]
    ret["inout"] = ret["in"] + ret["out"]
    

@tangelo.types(id=int, ids=json.loads, domain=json.loads)
@tangelo.return_type(bson.json_util.dumps)
def run(name=None, id=None, ids=None, domain=None, cluster=None):
    if domain:
        domain = set(domain)

    if name:
        r = requests.get('http://localhost:5000/site?name=%s' % str(name))
        ret = r.json()
        add_extras(ret)
        return r.json()
    #     doc = collection.find_one(
    #         {"_id": binary_search(collection_list, name)})
    #     doc["tldid"] = tld_offset[doc["tld"]] + doc["tldid"]
    #     if "out" not in doc:
    #         doc["out"] = []
    #         doc["in"] = []
    #         doc["inout"] = []
    #         docs = overflow_collection.find({"node": doc["_id"]})
    #         for d in docs:
    #             if "out" in d:
    #                 doc["out"] = doc["out"] + d["out"]
    #             if "inout" in d:
    #                 doc["inout"] = doc["inout"] + d["inout"]
    #             if "in" in d:
    #                 doc["in"] = doc["in"] + d["in"]
    #     return doc
    # # XXX Not being used
    # if id:
    #     return collection.find_one(id)
    # Subgraph operation
    if ids:
        r = requests.get("http://localhost:5000/subgraph?ids=%s" % ",".join(map(str, ids)))
        result = r.json()
        map(add_extras, result)
            

        # import scipy
        # import scipy.cluster.vq
        # import numpy
        # import operator

        ## result = requests.get_subgraph
        # result = [
        #     {
        #         "_id": d["_id"],
        #         "name": d["name"],
        #         "tld": d["tld"], # "com" or "net" ....
        #         # don't need this
        #         "tldid": tld_offset[d["tld"]] + d["tldid"],
        #          '''.com: 10000, .net:123 '''
        #         "out": (
        #             # Ignore this, checks domains are in subgraph
        #             [] if "out" not in d else (
        #                 list(domain.intersection(d["out"])) if domain else d["out"])
        #             )
        #     }
        #     for d in collection.find(
        #         {"_id": {"$in": ids}},
        #         fields=["out", "name", "tld", "tldid"]
        #     )
        # ]

        # idx = {}
        # for i in range(len(result)):
        #     idx[result[i]['_id']] = i

        # if cluster == "mcl":
        #     undirected = [{"_id": d["_id"], "out": [n for n in d["out"]]} for d in result]
        #     for d in undirected:
        #         for n in d["out"]:
        #             undirected[idx[n]]["out"].append(d["_id"])

        #     cluster = run_mcl_cluster(undirected)

        #     pairs = zip(result, cluster)
        #     sorted_pairs = sorted(pairs, key=operator.itemgetter(1))
        #     return [
        #         {
        #             "_id": d[0]["_id"],
        #             "out": d[0]["out"],
        #             "name": d[0]["name"],
        #             "tld": d[0]["tld"],
        #             "tldid": d[0]["tldid"],
        #             "cluster": d[1]
        #         }
        #         for d in sorted_pairs
        #     ]

        # if cluster == "kmeans":
        #     m = numpy.zeros((len(ids), len(ids)))

        #     for d in result:
        #         for r in d['out']:
        #             # m[idx[d['_id']]][idx[r]] = 1
        #             m[idx[d['_id']]][idx[r]] = 1

        #     centers, _ = scipy.cluster.vq.kmeans(m, 2)
        #     cluster, _ = scipy.cluster.vq.vq(m, centers)
        #     pairs = zip(result, cluster)
        #     sorted_pairs = sorted(pairs, key=operator.itemgetter(1))
        #     return [
        #         {
        #             "_id": d[0]["_id"],
        #             "out": d[0]["out"],
        #             "name": d[0]["name"],
        #             "tld": d[0]["tld"],
        #             "tldid": d[0]["tldid"],
        #             "cluster": d[1]
        #         }
        #         for d in sorted_pairs
        #     ]

        return result

    return []
