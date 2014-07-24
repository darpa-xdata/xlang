/*jslint nomen: true, browser: true, unparam: true */

(function ($, d3, vg) {
    "use strict";

    function getParameterByName(name) {
        name = name.replace(/[\[]/, "\\[").replace(/[\]]/, "\\]");
        var regex = new RegExp("[\\?&]" + name + "=([^&#]*)"),
            results = regex.exec(location.search);
        return results === null ? "" : decodeURIComponent(results[1].replace(/\+/g, " "));
    }

    $(function () {

        var site = getParameterByName("site"),

        // $(".search-form").submit(function () {
            data = {
                "matrix": [],
                "nodes": []
            },
            nodeMap = {},
            view,
            tlds = ["com", "de", "net", "org", "uk", "nl", "ru", "info", "it", "pl", "br", "au", "jp", "fr", "edu", "gov", "other"],
            counts = [
                21245514,
                2995130,
                2330354,
                2204897,
                1727006,
                1017671,
                750963,
                698283,
                661345,
                590775,
                570253,
                513838,
                511159,
                468399,
                419662,
                390970,
                363648,
                42889800 - 37459867
            ],
            tldMap = {},
            all = [],
            inMap = {},
            outMap = {},
            domain;

        if (site === "") {
            return;
        }

        $('.site').val(site);

        tlds.forEach(function (d) {
            tldMap[d] = true;
        });

        function renderMatrix(chart, el) {
            view = chart({el: el, data: data});
            view.on("mouseover", function (event, item) {
                // console.log(nodeMap[item.datum.data.source].name + ' - ' + nodeMap[item.datum.data.target].name);
                var id = item.datum.data.id,
                    incoming = domain.in.indexOf(id) !== -1,
                    outgoing = domain.out.indexOf(id) !== -1,
                    name = nodeMap[item.datum.data.id].name,
                    message;
                if (incoming && outgoing) {
                    message = domain.name + ' &lrarr; ' + name;
                } else if (incoming) {
                    message = domain.name + ' &larr; ' + name;
                } else if (outgoing) {
                    message = domain.name + ' &rarr; ' + name;
                } else {
                    message = name;
                }
                $('.current').html(message);
            });
            view.on("mouseout", function () {
                $('.current').html('&nbsp;');
            });
            view.on("click", function (event, item) {
                $('.site').val(nodeMap[item.datum.data.id].name);
                $('.search-form').submit();
            });
            view.update();
        }

        function renderOverview(overview, el, filter) {
            overview({el: el, data: {
                nodes: data.nodes.filter(filter)
            }})
                .on("mouseover", function (event, item) {
                    // console.log(nodeMap[item.datum.data.source].name + ' - ' + nodeMap[item.datum.data.target].name);
                    var id = item.datum.data.id,
                        name = nodeMap[id].name;
                    $('.current').html(name);
                })
                .on("mouseout", function () {
                    $('.current').html('&nbsp;');
                })
                .on("click", function (event, item) {
                    $('.site').val(nodeMap[item.datum.data.id].name);
                    $('.search-form').submit();
                })
                .update();
        }

        function renderOverviewBar(overviewBar, el, filter) {
            overviewBar({
                el: el,
                data: {
                    table: data.nodes.filter(filter)
                        .reduce(function (last, now) {
                            var index = tlds.indexOf(now.tld);
                            last[index].y += 1;
                            return last;
                        }, tlds.map(function (d) { return {x: d, y: 0}; }))
                        .map(function (d) {
                            var index = tlds.indexOf(d.x);
                            return {x: d.x, y: d.y / counts[index]};
                        })
                }
            }).update();
        }

        // parse a spec and create a visualization view
        vg.parse.spec("matrix.json", function (matrix) {
            vg.parse.spec("overview.json", function (overview) {
                vg.parse.spec("overview-bar.json", function (overviewBar) {
                    d3.json('nodes?name=' + site, function (error, d) {
                        domain = d;
                        data.nodes.push({id: d._id, tldid: d.tldid, cluster: null});
                        nodeMap[d._id] = d;
                        d.inout.forEach(function (a) {
                            if (!nodeMap[a]) {
                                all.push(a);
                                nodeMap[a] = true;
                                data.matrix.push({source: d._id, target: a, cluster: null, opacity: 1});
                                data.matrix.push({source: a, target: d._id, cluster: null, opacity: 1});
                            }
                        });
                        d.in.forEach(function (a) {
                            inMap[a] = true;
                            if (!nodeMap[a]) {
                                all.push(a);
                                nodeMap[a] = true;
                                data.matrix.push({source: a, target: d._id, cluster: null, opacity: 1});
                            }
                        });
                        d.out.forEach(function (a) {
                            outMap[a] = true;
                            if (!nodeMap[a]) {
                                all.push(a);
                                nodeMap[a] = true;
                                data.matrix.push({source: d._id, target: a, cluster: null, opacity: 1});
                            }
                        });
                        d3.json('nodes?cluster=mcl&ids=' + JSON.stringify(all) + '&domain=' + JSON.stringify(all), function (error, nodes) {
                            nodes.forEach(function (node) {
                                data.nodes.push({id: node._id, tld: node.tld, tldid: node.tldid, cluster: node.cluster});
                                nodeMap[node._id] = node;
                            });
                            nodes.forEach(function (node) {
                                node.out.forEach(function (n) {
                                    var cluster;
                                    if (nodeMap[n]) {
                                        cluster = nodeMap[n].cluster;
                                        cluster = (cluster === node.cluster) ? cluster : null;
                                        data.matrix.push({source: node._id, target: n, cluster: cluster, opacity: cluster === null ? 0.2 : 1});
                                    }
                                });
                            });

                            data.nodes.forEach(function (d) {
                                d.tld = nodeMap[d.id].tld;
                                if (!tldMap[d.tld]) {
                                    d.tld = "other";
                                }
                                d.row = Math.floor(d.tldid / 6550);
                                d.col = d.tldid % 6550;
                            });

                            data.matrix.forEach(function (d) {
                                d.sourcetld = nodeMap[d.source].tld;
                                if (!tldMap[d.sourcetld]) {
                                    d.sourcetld = "other";
                                }
                            });

                            renderMatrix(matrix, "#vis");
                            renderOverview(overview, "#overview-in", function (d) {
                                return inMap[d.id] === true;
                            });
                            renderOverview(overview, "#overview-out", function (d) {
                                return outMap[d.id] === true;
                            });
                            renderOverview(overview, "#overview-inout", function (d) {
                                return inMap[d.id] === true && outMap[d.id] === true;
                            });
                            renderOverviewBar(overviewBar, "#bar-in", function (d) {
                                return inMap[d.id] === true;
                            });
                            renderOverviewBar(overviewBar, "#bar-out", function (d) {
                                return outMap[d.id] === true;
                            });
                            renderOverviewBar(overviewBar, "#bar-inout", function (d) {
                                return inMap[d.id] === true && outMap[d.id] === true;
                            });
                        });
                    });
                });
            });
        });
    });

}(window.$, window.d3, window.vg));
