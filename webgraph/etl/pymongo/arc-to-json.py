import array
import csv
import json
import pymongo

nodefile = 'pld-index'
arcfile = 'pld-arc'
collection = 'wdc.pld'
overflow = 'wdc.pld.overflow'
group = 10000
maxlinks = 500000

# nodefile = 'pld-index-sample'
# arcfile = 'pld-arc-sample'
# collection = 'wdc.pld.sample'
# overflow = 'wdc.pld.sample.overflow'
# group = 10
# maxlinks = 2


# class ArrayEncoder(json.JSONEncoder):

#     def default(self, obj):
#         if isinstance(obj, array.array):
#             return obj.tolist()
#         # Let the base class default method raise the TypeError
#         return json.JSONEncoder.default(self, obj)

names = []
outgoing = []
incoming = []
r = csv.reader(open(nodefile), delimiter='\t')
n = 0
for d in r:
    names.append(d[0])
    incoming.append(array.array('i'))
    outgoing.append(array.array('i'))
    n += 1
    if n % group == 0:
        print n, 'nodes'

r = csv.reader(open(arcfile), delimiter='\t')
n = 0
for d in r:
    source = int(d[0])
    target = int(d[1])
    incoming[target].append(source)
    outgoing[source].append(target)
    n += 1
    if n % group == 0:
        print n, 'arcs'

# f = open(outfile, 'w')
# json.dump(outgoing, open(outfile, 'w'), cls=ArrayEncoder)

c = pymongo.MongoClient()['xdata'][collection]
over = pymongo.MongoClient()['xdata'][overflow]
c.drop()
over.drop()

batch = []
for i in xrange(len(names)):
    inout = list(set(incoming[i]).intersection(outgoing[i]))
    doc = {'_id': i, 'name': names[i], 'iodeg': len(inout), 'indeg': len(incoming[i]), 'outdeg': len(outgoing[i])}
    if len(incoming[i]) + len(outgoing[i]) + len(inout) > maxlinks:
        print 'splitting', names[i]
        for j in range(len(incoming[i])/maxlinks + 1):
            part = incoming[i][j*maxlinks:(j+1)*maxlinks]
            if len(part) > 0:
                over.insert({'node': i, 'in': part.tolist()})
        for j in range(len(outgoing[i])/maxlinks + 1):
            part = outgoing[i][j*maxlinks:(j+1)*maxlinks]
            if len(part) > 0:
                over.insert({'node': i, 'out': part.tolist()})
        for j in range(len(inout)/maxlinks + 1):
            part = inout[j*maxlinks:(j+1)*maxlinks]
            if len(part) > 0:
                over.insert({'node': i, 'inout': part})
    else:
        doc['in'] = incoming[i].tolist()
        doc['out'] = outgoing[i].tolist()
        doc['inout'] = inout
    batch.append(doc)
    if i % group == 0:
        c.insert(batch)
        batch = []
        print i, 'nodes'

c.insert(batch)
