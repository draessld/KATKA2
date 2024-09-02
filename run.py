# -*- coding: utf-8 -*-

import argparse
import subprocess
import os
from Bio import Phylo
from Bio import Seq
from Bio import SeqIO
import requests as r
import xml.etree.ElementTree as ET
from tqdm import tqdm
import json
import sys
import shutil
import glob
import re

#   TODO
#       pipelining commands??

#   GLOBALS
allowed_fasta_extenstions = ['.fa','.fasta','.fna']
build_options = ['all','base','minimize','kernelize']
cfg_file=os.path.join(os.path.dirname(__file__),'configure.json')

#   Help functions
def save_metadata(**kwargs):
    try:
        with open(cfg_file, 'w') as file:
            json.dump(kwargs, file, indent=4)
    except IOError as e:
        raise Exception(f"Failed to save configuration: {e}")

#   Basic functions
def setup(func):
    def wrapper(*args, **kwargs):
        try:
            with open(cfg_file,'r') as f:
                kwargs = json.load(f)
            kwargs["configure"]
        except Exception as e:
            raise Exception(f"Missing configuration: please configure your program with valid data first, {e}")
        
        return func(*args, **kwargs)
    
    return wrapper

def get_mem_stats(func):
    def wrapper(*args, **kwargs):
        output = func(*args, **kwargs)
        for line in output.split('\n'):
            pass
        return output
    return wrapper

def get_longest_lr_mem(func):
    def wrapper(*args, **kwargs):
        output = func(*args, **kwargs)
        for line in output.split('\n'):
            pass
        return output
    return wrapper

@setup
def build(*args, **kwargs):
    """ Build index """
    files = args[0]
    executable_path = kwargs["cpp_exe"]["build_exe"]
    for file in files:
        index_dir = file+'.index'
        print("Building",file,index_dir)
        p = subprocess.Popen([executable_path,file,index_dir], stdout=subprocess.PIPE)
        out, err = p.communicate()
        yield out.decode(),index_dir
        # yield "out",index_dir

@setup
def find(*args, **kwargs):
    """ Find MEM tables """
    index = args[0]
    all = args[1]
    pattern = args[2]
    try:
        executable_path = kwargs["cpp_exe"]["find_exe"]
        if os.path.exists(executable_path) and os.access(executable_path, os.X_OK):
            p = subprocess.Popen([executable_path,index,str(all),pattern], stdout=subprocess.PIPE)
            out, err = p.communicate()
            return out.decode()
    except Exception as e:
        raise Exception(f"Index wasnt found, build it first with comand *build* {e}")

def patterns(*args):
    patterns = list()
    for pattern in args[0]:
        if os.path.exists(pattern):
            with open(pattern, 'r') as f:
                for p in f.read().splitlines():
                    yield p
                # patterns += f.read().splitlines()
        else:
            yield pattern
            # patterns.append(pattern)

@setup
def indices(*args, **kwargs):
    for f in os.listdir(kwargs["output_dir"]):
        if f.endswith('.index'):
            yield os.path.join(kwargs["output_dir"],f)

@setup
def minimize(*args, **kwargs):
    """ create minimized version of data, save entry with parameters on special output folder """
    files = args[0]
    m = str(args[1])
    w = str(args[2])

    if not 'minimize' in kwargs:
        minimize_d = dict()
        minimize_d['w'] = list()
        minimize_d['m'] = list()
        kwargs['minimize'] = minimize_d

    executable_path = kwargs["cpp_exe"]["minimize_exe"]
    if not os.path.exists(executable_path) or  not os.access(executable_path, os.X_OK):
        raise Exception("Unable to find executable: {executable_path}")
        
    out_files = list()
    for file in files:
        out_file=file+f'_m{m}_w{w}'
        out_files.append(out_file)
        try:    
            with open(file,'rb') as f:
                p = subprocess.run([executable_path,m,w,f.read()], stdout=open(out_file,'wb'),stderr=subprocess.PIPE)
        except Exception as e:
            raise Exception(f'Error during minimization')

    if w not in kwargs['minimize']['w']:
        kwargs['minimize']['w'].append(w)
    if m not in kwargs['minimize']['m']:
        kwargs['minimize']['m'].append(m)

    save_metadata(**kwargs)
    return out_files
    
@setup
def kernelize(*args, **kwargs):
    """ create katka kernel of base data, save entry with parameters on special output folder """
    files = args[0]
    k = str(args[1])

    if not 'kernelize' in kwargs:
        kernelize_d = dict()
        kernelize_d['k'] = list()
        kwargs['kernelize'] = kernelize_d

    executable_path = kwargs["cpp_exe"]["kernelize_exe"]
    if not os.path.exists(executable_path) or  not os.access(executable_path, os.X_OK):
        raise Exception("Unable to find executable: {executable_path}")
        
    out_files = list()
    for file in files:
        out_file = file+f'_k{k}'
        out_files.append(out_file)
        try:    
            with open(file,'rb') as f:
                p = subprocess.run([executable_path,k,f.read()], stdout=open(out_file,'wb'),stderr=subprocess.PIPE)
        except Exception as e:
            raise Exception(f'Error during kernelization') # problem with decoding minimization

    if k not in kwargs['kernelize']['k']:
        kwargs['kernelize']['k'].append(k)

    save_metadata(**kwargs)
    return out_files

def clean():
    data=None
    if os.path.exists(cfg_file):
        with open(cfg_file,'r') as f:
            data = json.load(f)
        
    if os.path.exists(data["output_dir"]):
        shutil.rmtree(data["output_dir"])

    if "configure" in data:
        del data["configure"]   
    if "minimize" in data:
        del data["minimize"]    
    if "kernelize" in data:
        del data["kernelize"]   

def extract_singleton(input):
    """"""
    it = input.split('\n')
    for name,line in zip(it[0::2], it[1::2]):
        pattern, time, nmems = name.split('\t')
        pattern = pattern[1:]
        p = r'\[(\d+),(\d+)\]\{(\d+),(\d+)\}'
        max = (0,-1)
        for mem in line.split('\t'):
            match = re.search(p,mem)
            if match :
                sp,ep,lg,rg = [int(i) for i in match.groups()]
                # print(pattern[sp:ep],sp,ep,lg,rg,ep-sp)
                if lg == rg and (ep-sp) > max[0]:
                    max = (ep-sp,lg)
    return max[1]

def configure(**kwargs): 
    #   remove old metadata file (with slogan about removing)
    # if os.path.exists(cfg_file):
    #     print("Warning: Replacing previous configuration!")
    #     os.remove(cfg_file)

    with open(cfg_file,'r') as f:
        cfg = json.load(f)

    #   validate output folder
    if kwargs["output_dir"] != None:
        cfg["output_dir"] = kwargs["output_dir"]

    if not os.path.exists(cfg["output_dir"]):
        os.mkdir(cfg["output_dir"])

    configure_d = dict()
    configure_d["tree_file"] = kwargs["tree_file"] 
    configure_d["data_dir"] = kwargs["data_dir"] 

    #   read tree
    tree = Phylo.read(kwargs["tree_file"], "newick")
    leaves = tree.get_terminals()
    configure_d["n_leaves"] = len(leaves) 

    project_name = os.path.basename(kwargs["tree_file"])[:-4]
    configure_d["project_name"] = project_name

    base_file = os.path.join(cfg["output_dir"], project_name)
    #   merge sequences
    fa_files = glob.glob(os.path.join(kwargs["data_dir"], "*.fa"))
    write_file = open(base_file,'w')
    for i,leaf in enumerate(leaves):
        try:
            #   try to find corresponding fasta file
            sequences = SeqIO.parse(os.path.join(kwargs["data_dir"],leaf.name+'.fa'),'fasta')
            concatenated_sequences = '#'.join(str(record.seq) for record in sequences)
            write_file.write(concatenated_sequences[1:])
            write_file.write('$')
        except Exception as e:
            raise Exception(f"Unable to proccess leaf: {leaf.name}, {e}")
            
    configure_d["base_file"] = base_file
    cfg["configure"] = configure_d

    # try:
    #     #   get filename
    #     filename,ext=kwargs["tree_file"].split('/')[-1].split('.')
    #     if ext != "nwk":
    #         raise ValueError("Unexpected extension of Newick format")

    #     #   read tree
    #     tree = Phylo.read(kwargs["tree_file"], "newick")
    #     leaves = tree.get_terminals()

    #     kwargs["data_path"] = "/home/draesdom/Projects/template/test_parse_download.fa"
    #     #   find data or download
    #     if "data_path" not in kwargs:
    #         #   try to find them with the same name as the tree but with another extension
    #         answer = input("Tree data was not found. Do you wish to download them from NCBI based on the leave's ID? Y/N\n")
    #         if answer in ['y','Y','yes','Yes','YES']:
    #             kwargs["data_path"] = download_data(leaves, filepath=os.path.join(kwargs["output_dir"],filename+"_data"))
    #             print(f'Data download succesfull, result on {kwargs["data_path"]}')
    #         else: 
    #             answer = input("Please specify filepath to FASTA format with matching sequence IDs\n")
    #             if not os.path.exists(answer):
    #                 raise FileNotFoundError("FASTA file was not found")
    #             elif ('.'+answer.split('.')[-1]) not in allowed_fasta_extenstions:
    #                 raise Exception(f"Unknown data format. Data file needs to be in FASTA format with following extenstions: {allowed_fasta_extenstions}")
    #             else:
    #                 kwargs["data_path"]=answer
    #                 print(f"Data was found on {answer}")
    #    
    # except IOError as e:
    #     raise Exception(f"Failed to get data: {e}")

    #   save configuration
    save_metadata(**cfg)
    print(f"Configuration successfull")

@setup
def download(*args, **kwargs):
    tree_file = args[0]
    database = args[1]

    project_name = os.path.basename(tree_file)[:-4]

    tree = Phylo.read(tree_file, "newick")
    leaves = tree.get_terminals()

    data_dir = os.path.join(kwargs["output_dir"],project_name+'_fasta')

    if not os.path.exists(data_dir):
        os.makedirs(data_dir)

    for leaf in tqdm(leaves):
        with open(os.path.join(data_dir,leaf.name+".fa"),'w') as f:
            f.write('>'+leaf.name+'\n')
            seq = download_sequence(leaf.name,db=database)
            i=0
            while i+60 < len(seq):
                f.write(seq[i:i+60]+'\n')
                i+=60
            f.write(seq[i:]+'\n')

    print(f'Data download succesfull, result on {data_dir}')

def download_sequence(name,db='protein'):
    base = 'https://eutils.ncbi.nlm.nih.gov/entrez/eutils/'
    name = name.split(':')[0]
    url = base + f"esearch.fcgi?db={db}&term={name}&usehistory=y"
    output = r.get(url)
    xml = ET.fromstring(output.text)
    QueryKey = xml.find("QueryKey").text
    WebEnv = xml.find("WebEnv").text
    x = r.get(base +  f"efetch.fcgi?db={db}&query_key={QueryKey}&WebEnv={WebEnv}&rettype=fasta&retmode=text")
    return ''.join(x.text.split('\n')[1:])

def read_nwk(nwk):
    tree = Phylo.read(nwk, "newick")
    leaves = tree.get_terminals()
    # print(f'Total {len(leaves)} sequences')
    return tree,leaves

    with open("./input.txt",'w') as f:
        for leaf in tqdm(leaves):
            f.write(download_sequence(leaf.name)+'$')
    #   run mem finding
    try:
        subprocess.run("/home/draesdom/Projects/KATKA2/build/index-build -i/home/draesdom/Projects/template/input.txt", check=True)
        # subprocess.run(["./" + executable], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error running index-build: {e}")
    # r,l = 2,4

    #   get lca
    # lca = tree.common_ancestor(leaves[r],leaves[l])

    # print(lca)
    #   draw lca
    # Phylo.draw_ascii(lca)

def parse_range(value):
    try:
        if value == None:
            return list()
        nums = [int(i) for i in value.split('-')]
        if len(nums) == 1:
            return nums
        else:
            return range(*nums)
    except Exception as e:
        raise argparse.ArgumentTypeError(f"Invalid input: {value}. Must be a single number or a range in the format 'start-end'.")

@setup
def get_base_file(*args,**kwargs):
    return kwargs["configure"]["base_file"]

def list_indices(*args, **kwargs):
    for index in indices():
        print(index)

def main():
    #   parse arguments
    parser = argparse.ArgumentParser(description="Classify read in the tree")
    # parser.add_argument("command", help="The command to execute (e.g., build, download, get, find, minimize, kernelize).")
    subparsers = parser.add_subparsers(dest="command", help="Command to execute")

    # Clean command
    subparsers.add_parser("clean", help="Clean your workspace")

    # Download command
    parser_download = subparsers.add_parser("download", help="Download data in .nwk tree from NCBI database")
    parser_download.add_argument("tree_file", type=str, help="Path to the tree Newick format, leaves contains sequence names")
    parser_download.add_argument("database", type=str, help="Path to the tree Newick format, leaves contains sequence names")

    # configure command
    parser_configure = subparsers.add_parser("configure", help="Configure your workspace with tree file in Newick format")
    parser_configure.add_argument("tree_file", type=str, help="Path to the tree Newick format, leaves contains sequence names")
    parser_configure.add_argument("data_dir", type=str, help="Path to the sequences in the taxonomic tree")
    parser_configure.add_argument("--output_dir","-o",help="Path to the output folder (default is current directory).")

    # Minimize command
    parser_minimize = subparsers.add_parser("minimize", help="digest original text with minimizers")
    parser_minimize.add_argument("m", type=parse_range, help="kernel size (5 - 30), can be given range in format a-b-c as from a to b by c steps")
    parser_minimize.add_argument("w", type=parse_range, help="minimizer window size (10 - 50), can be given range in format a-b-c as from a to b by c steps")

    # Kernelize command
    parser_kernelize = subparsers.add_parser("kernelize", help="From original file create its KATKA kernel")
    parser_kernelize.add_argument("k", type=int, help="k-mer length (5 - 500),can be given range in format a-b-c as from a to b by c steps")
    

    # List command
    subparsers.add_parser("list", help="list all built indices")

    # Build command
    parser_build = subparsers.add_parser("build", help="Build the project")
    parser_build.add_argument("-m", type=parse_range, required = False, default = list(), help="kernel size (5 - 30), can be given range in format a-b-c as from a to b by c steps")
    parser_build.add_argument("-w", type=parse_range, required = False, default = list(), help="minimizer window size (10 - 50), can be given range in format a-b-c as from a to b by c steps")
    parser_build.add_argument("-k", type=parse_range, required = False, default = list(), help="k-mer length (5 - 500),can be given range in format a-b-c as from a to b by c steps")

    # find command
    parser_find = subparsers.add_parser("find", help="For every pattern returns the best leaf match (longest singleton MEM)")
    parser_find.add_argument("index", type=str, help="path to the index directory")
    parser_find.add_argument("patterns", nargs='+', type=str, help="Single pattern or a pattern file (one pattern per line)")

    # findall command
    parser_findall = subparsers.add_parser("findall", help="find MEMs of given patterns with respect to the tree data")
    parser_findall.add_argument("index", type=str, help="path to the index directory")
    parser_findall.add_argument("patterns", nargs='+', type=str, help="Single pattern or a pattern file (one pattern per line)")

    # clasify command
    parser_find = subparsers.add_parser("clasify", help="find MEMs of given patterns with respect to the tree data")
    # parser_find.add_argument("patterns", nargs='+', type=str, help="Single pattern or a pattern file (one pattern per line)")

    args = parser.parse_args()

    if not args.command:
        parser.print_help()
        sys.exit(1)

    if args.command == "configure": 
        configure(**vars(args))
    elif args.command == "download":   # done
        download(args.tree_file, args.database)
    elif args.command == "clean":   # done
        clean()
    elif args.command == "list":   # done
        list_indices()
    elif args.command == "kernelize":   # done
        kernelize(args.k)
    elif args.command == "findall":
        for pattern in patterns(args.patterns):
            out = find(args.index,1,pattern)
            print(out)
    elif args.command == "find":
        for pattern in patterns(args.patterns):
            out = find(args.index,0,pattern)
            print(out) 
    elif args.command == "minimize":
        files = [get_base_file()]
        for m in args.m:
            for w in args.w:
                minimize(files,m,w)           
    elif args.command == "build":
        files = [get_base_file()]
        min_files = list()
        for m in args.m:
            for w in args.w:
                min_files += minimize(files,m,w)        

        if min_files != list():
            files = min_files

        ker_files = list()
        for k in args.k:
            ker_files += kernelize(files,k)

        if ker_files != list():
            files = ker_files

        for out,index_dir in build(files):
            print(out)
    elif args.command == "clasify":
        raise Exception("Not Implemented yet")
    else:
        print(f"Unknown command: {args.command}")

if __name__ == "__main__":
    main()
