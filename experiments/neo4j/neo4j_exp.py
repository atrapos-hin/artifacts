import sys
import time

def get_cypher(query):
	parts = query.split("\t")
	metapath = parts[0]
	condition = parts[1]

	cypher = 'MATCH ';
	condition_index = len(metapath)-1

	for i in range(len(metapath)):
		# print(e)
		if (i+1 <= len(metapath)-1):
			if cypher == 'MATCH ':
				cypher += '(' + metapath[i].lower() + str(i) + ':' + metapath[i] + ')'
				if condition_index == len(metapath)-1 and metapath[i] == condition[0]:
					condition_index = i

			# edge = '(' + metapath[i].lower() + ':' + metapath[i] + ')-[:' 
			# + metapath[i] + ']->(' +
			if condition_index == len(metapath)-1 and metapath[i] == condition[0]:
					condition_index = i
			edge = '-[:' + metapath[i] + metapath[i+1] + ']->(' + metapath[i+1].lower() + str(i+1)+ ':' + metapath[i+1] + ')' 
			cypher += edge

	condition = condition.replace('"', '')
	filter_id = condition.split('=')[1]
	first = metapath[0].lower()
	last =metapath[-1].lower()

	cypher += ' WHERE ' + condition[0].lower() + str(condition_index) + '.id=\'' + str(filter_id) + '\''
	cypher += ' RETURN ' + first + '0.id, ' + last + str(len(metapath)-1) + '.id, count(*)'

	return cypher

# q = 'CPA	A.id="31659"'
# print(get_cypher(q))
workload = sys.argv[1]
total = 0
cnt = 0
from neo4j import GraphDatabase
driver = GraphDatabase.driver('bolt://localhost:7687', auth=('neo4j', 'cqg2sCQWZ96tDiYCqhDa'), encrypted=False)
total_time = 0
with driver.session() as session:
	with session.begin_transaction() as tx:
		with open(workload) as fp:
			query = fp.readline()
			while query:
				query = query.rstrip()
				cypher_query = get_cypher(query)
				#cypher_query = "MATCH(a:A)-[:AP]->(p:P) WHERE a._id <= 10000RETURN a._id, p_id"
				#cypher_query = "match(p1:P)-[:PC]->(c:C)-[:CP]->(p2:P)-[:PP]->(p3:P) where c._id = 1239 WITH p1, p3, count(*) as count RETURN p1._id, p3._id, count;"
				#print(cypher_query)
				start = time.time()
				result = tx.run(cypher_query)
				records = list(result)  # consume the records
				total_time += time.time() - start
				#print(result.consume())
				#avail = result.consume().result_available_after
				#cons = result.consume().result_consumed_after
				#print(avail)
				#print(cons)
				#total_time = avail + cons
				#print(query + "\t" + str(total_time), flush=True)
				#total += total_time
				cnt += 1

				query = fp.readline()

		tx.close()
#print(cnt)
sys.stderr.write(workload + "\t" + str(total_time))

