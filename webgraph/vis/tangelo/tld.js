// Compute TLD from the MongoDB shell
db.wdc.pld.find().forEach(function (d) { var p = d.name.split("."); d.tld = p[p.length - 1]; db.wdc.pld.tld.insert(d); });

// Compute TLD index from the MongoDB shell
db.wdc.pld.find().forEach(function (d) { var p = d.name.split("."); d.tld = p[p.length - 1]; if (tlds[d.tld] === undefined) tlds[d.tld] = 0; d.tldid = tlds[d.tld]; tlds[d.tld] += 1; db.wdc.pld.tldind.insert(d); });

// Compute TLD counts and store in wdc.tld
db.wdc.pld.mapReduce(function () { var p = this.name.split("."), tld = p[p.length - 1]; emit(tld, 1); }, function (key, values) { return Array.sum(values); }, {out: "wdc.tld"});
