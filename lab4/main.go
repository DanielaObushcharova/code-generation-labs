package main

import (
	"fmt"
)

type variable struct {
	name    string
	isSSA   bool
	version int
}

func (v *variable) makeSSA(version int) {
	v.isSSA = true
	v.version = version
}

func (v *variable) toString() string {
	if v.isSSA {
		return fmt.Sprintf("%v(%v)", v.name, v.version)
	}
	return v.name
}

type instruction struct {
	lhs  variable
	deps []variable
}

func (in *instruction) toString(fname string) string {
	res := fmt.Sprintf("%v = %v(", in.lhs.toString(), fname)
	for i, s := range in.deps {
		res += s.toString()
		if i != len(in.deps)-1 {
			res += ","
		}
	}
	res += ")"
	return res
}

type basicBlock struct {
	id           int
	phis         map[string]instruction
	instructions []instruction
	preds        []*basicBlock
	succs        []*basicBlock
}

func (bb *basicBlock) addSucc(other *basicBlock) {
	bb.succs = append(bb.succs, other)
	other.preds = append(other.preds, bb)
}

func (bb *basicBlock) toString() string {
	enumGraph(bb)
	used := make(map[*basicBlock]struct{})
	return bb.toStringRec(used, true)
}

func (bb *basicBlock) toStringRec(used map[*basicBlock]struct{}, header bool) string {
	used[bb] = struct{}{}
	res := ""
	if header {
		res += "digraph G {\nnode[shape=rectangle]\n"
	}
	text := ""
	for _, u := range bb.phis {
		text += u.toString("phi") + "\\n"
	}
	for _, u := range bb.instructions {
		text += u.toString("f") + "\\n"
	}
	res += fmt.Sprintf("%v [label=\"%v\"]\n", bb.id, text)
	for _, u := range bb.succs {
		res += fmt.Sprintf("%v -> %v\n", bb.id, u.id)
		if _, ok := used[u]; !ok {
			res += u.toStringRec(used, false) + "\n"
		}
	}
	if header {
		res += "}"
	}
	return res
}

func enumGraph(root *basicBlock) {
	cnt := 1
	enumGraphRec(root, &cnt)
}

func enumGraphRec(root *basicBlock, cnt *int) {
	root.id = *cnt
	*cnt++
	for _, u := range root.succs {
		if u.id == 0 {
			enumGraphRec(u, cnt)
		}
	}
}

func makeSSA(root *basicBlock) *basicBlock {
	visited := make(map[*basicBlock]struct{})
	vars := make(map[string]struct{})
	getAllVars(root, visited, vars)
	newRoot := &basicBlock{
		instructions: nil,
	}
	newRoot.addSucc(root)
	for v := range vars {
		newRoot.instructions = append(newRoot.instructions, instruction{
			lhs: variable{
				name: v,
			},
		})
	}
	all := make(map[*basicBlock]struct{})
	getAllBasicBlocks(newRoot, all)
	idoms := getImmediateDominators(newRoot, all)
	for v := range vars {
		placePhis(newRoot, v, all, idoms)
		addVersions(newRoot, v, idoms)
	}
	return newRoot
}

func getAllVars(root *basicBlock, visited map[*basicBlock]struct{}, vars map[string]struct{}) {
	visited[root] = struct{}{}
	for _, s := range root.instructions {
		vars[s.lhs.name] = struct{}{}
		for _, dep := range s.deps {
			vars[dep.name] = struct{}{}
		}
	}
	for _, u := range root.succs {
		if _, ok := visited[u]; !ok {
			getAllVars(u, visited, vars)
		}
	}
}

func placePhis(root *basicBlock, name string, all map[*basicBlock]struct{}, idoms map[*basicBlock]*basicBlock) {
	front := getDominanceFrontier(all, idoms)
	visited := make(map[*basicBlock]struct{})
	defs := make(map[*basicBlock]struct{})
	findDefinitions(root, name, visited, defs)
	var queue []*basicBlock
	for u := range defs {
		queue = append(queue, u)
	}
	visited = make(map[*basicBlock]struct{})
	for len(queue) != 0 {
		u := queue[0]
		queue = queue[1:]
		for v := range front[u] {
			if _, ok := visited[v]; !ok {
				visited[u] = struct{}{}
				if v.phis == nil {
					v.phis = make(map[string]instruction)
				}
				v.phis[name] = instruction{
					lhs: variable{
						name: name,
					},
					deps: make([]variable, len(v.preds)),
				}
				for i := 0; i < len(v.preds); i++ {
					v.phis[name].deps[i].name = name
				}
				queue = append(queue, v)
			}
		}
	}
}

func addVersions(root *basicBlock, name string, idoms map[*basicBlock]*basicBlock) {
	counter := 0
	var stack []int
	children := make(map[*basicBlock][]*basicBlock)
	for u, v := range idoms {
		children[v] = append(children[v], u)
	}
	visited := make(map[*basicBlock]struct{})
	traverseVersions(root, name, &counter, &stack, children, visited)
}

func traverseVersions(root *basicBlock, name string, counter *int, stack *[]int, children map[*basicBlock][]*basicBlock, visited map[*basicBlock]struct{}) {
	visited[root] = struct{}{}
	if v, ok := root.phis[name]; ok {
		v.lhs.makeSSA(*counter)
		root.phis[name] = v
		*stack = append(*stack, *counter)
		*counter += 1
	}
	for i, s := range root.instructions {
		for j := range s.deps {
			if s.deps[j].name == name {
				root.instructions[i].deps[j].makeSSA(getLast(*stack))
			}
		}
		if s.lhs.name == name {
			root.instructions[i].lhs.makeSSA(*counter)
			*stack = append(*stack, *counter)
			*counter += 1
		}
	}
	for _, u := range root.succs {
		if _, ok := u.phis[name]; ok {
			j := whichPredecessor(u, root)
			s := u.phis[name]
			s.deps[j].makeSSA(getLast(*stack))
			u.phis[name] = s
		}
	}

	for _, u := range children[root] {
		if _, ok := visited[u]; !ok {
			traverseVersions(u, name, counter, stack, children, visited)
		}
	}

	for _, s := range root.instructions {
		if s.lhs.name == name {
			*stack = (*stack)[:len(*stack)-1]
		}
	}
}

func whichPredecessor(succ, pred *basicBlock) int {
	for i, p := range succ.preds {
		if p == pred {
			return i
		}
	}
	return -1
}

func findDefinitions(root *basicBlock, name string, visited, result map[*basicBlock]struct{}) {
	visited[root] = struct{}{}
	for _, s := range root.instructions {
		if s.lhs.name == name {
			result[root] = struct{}{}
			break
		}
	}
	for _, c := range root.succs {
		if _, ok := visited[c]; !ok {
			findDefinitions(c, name, visited, result)
		}
	}
}

func getDominanceFrontier(all map[*basicBlock]struct{}, idoms map[*basicBlock]*basicBlock) map[*basicBlock]map[*basicBlock]struct{} {
	res := make(map[*basicBlock]map[*basicBlock]struct{})
	for b := range all {
		res[b] = make(map[*basicBlock]struct{})
	}
	for b := range all {
		if len(b.preds) >= 2 {
			for _, p := range b.preds {
				runner := p
				for runner != idoms[b] {
					res[runner][b] = struct{}{}
					runner = idoms[runner]
				}
			}
		}
	}
	return res
}

func getImmediateDominators(root *basicBlock, all map[*basicBlock]struct{}) map[*basicBlock]*basicBlock {
	doms := getDominators(root, all)
	res := make(map[*basicBlock]*basicBlock)
	for u := range all {
		res[u] = root
	}
	for u := range all {
		for v := range doms[u] {
			if u == v {
				continue
			}
			if _, ok := doms[v][res[u]]; ok {
				res[u] = v
			}
		}
	}
	return res
}

func getDominators(root *basicBlock, all map[*basicBlock]struct{}) map[*basicBlock]map[*basicBlock]struct{} {
	res := make(map[*basicBlock]map[*basicBlock]struct{})
	for u := range all {
		res[u] = copySet(all)
	}
	res[root] = makeSingletonSet(root)
	changed := true
	for changed {
		changed = false
		for u := range all {
			udom := make(map[*basicBlock]struct{})
			if len(u.preds) > 0 {
				udom = copySet(res[u.preds[0]])
				for i := 1; i < len(u.preds); i++ {
					udom = intersectSets(udom, res[u.preds[i]])
				}
			}
			udom[u] = struct{}{}
			changed = changed || !areSetsEqual(res[u], udom)
			res[u] = udom
		}
	}
	return res
}

func getAllBasicBlocks(root *basicBlock, res map[*basicBlock]struct{}) {
	res[root] = struct{}{}
	for _, s := range root.succs {
		if _, ok := res[s]; !ok {
			getAllBasicBlocks(s, res)
		}
	}
}

func makeSingletonSet[T comparable](a T) map[T]struct{} {
	return map[T]struct{}{
		a: {},
	}
}

func intersectSets[T comparable](first, second map[T]struct{}) map[T]struct{} {
	res := make(map[T]struct{})
	for v := range first {
		if _, ok := second[v]; ok {
			res[v] = struct{}{}
		}
	}
	return res
}

func areSetsEqual[T comparable](first, second map[T]struct{}) bool {
	for v := range first {
		if _, ok := second[v]; !ok {
			return false
		}
	}
	for v := range second {
		if _, ok := first[v]; !ok {
			return false
		}
	}
	return true
}

func copySet[T comparable](s map[T]struct{}) map[T]struct{} {
	res := make(map[T]struct{})
	for v := range s {
		res[v] = struct{}{}
	}
	return res
}

func getLast[T any](slice []T) T {
	return slice[len(slice)-1]
}

func buildCfgExample1() *basicBlock {
	s1 := &basicBlock{
		instructions: []instruction{
			{
				lhs: variable{
					name: "x",
				},
			},
			{
				lhs: variable{
					name: "x",
				},
				deps: []variable{
					{
						name: "x",
					},
				},
			},
		},
	}
	s2 := &basicBlock{
		instructions: []instruction{
			{
				lhs: variable{
					name: "y",
				},
				deps: []variable{
					{
						name: "x",
					},
				},
			},
			{
				lhs: variable{
					name: "w",
				},
				deps: []variable{
					{
						name: "y",
					},
				},
			},
		},
	}
	s3 := &basicBlock{
		instructions: []instruction{
			{
				lhs: variable{
					name: "y",
				},
				deps: []variable{
					{
						name: "x",
					},
				},
			},
		},
	}
	s4 := &basicBlock{
		instructions: []instruction{
			{
				lhs: variable{
					name: "w",
				},
				deps: []variable{
					{
						name: "x",
					},
					{
						name: "y",
					},
				},
			},
			{
				lhs: variable{
					name: "z",
				},
				deps: []variable{
					{
						name: "x",
					},
					{
						name: "y",
					},
				},
			},
		},
	}
	s1.addSucc(s2)
	s1.addSucc(s3)
	s2.addSucc(s4)
	s3.addSucc(s4)
	return s1
}

func buildCfgExample2() *basicBlock {
	s1 := &basicBlock{
		instructions: []instruction{
			{
				lhs: variable{
					name: "i",
				},
			},
		},
	}
	s2 := &basicBlock{
		instructions: []instruction{
			{
				lhs: variable{
					name: "c",
				},
				deps: []variable{
					{
						name: "i",
					},
				},
			},
		},
	}
	s3 := &basicBlock{
		instructions: []instruction{
			{
				lhs: variable{
					name: "i",
				},
				deps: []variable{
					{
						name: "i",
					},
				},
			},
		},
	}
	s4 := &basicBlock{
		instructions: []instruction{
			{
				lhs: variable{
					name: "r",
				},
				deps: []variable{
					{
						name: "i",
					},
				},
			},
		},
	}
	s1.addSucc(s2)
	s2.addSucc(s3)
	s3.addSucc(s2)
	s2.addSucc(s4)
	return s1
}

func main() {
	root := buildCfgExample2()
	newRoot := makeSSA(root)
	fmt.Println(newRoot.toString())
}
