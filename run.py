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
import math
import datetime

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

def build(*args, **kwargs):
    """ Build index """
    files = args[0]
    executable_path = get_exe("build_exe")
    for file in files:
        index_dir = file+'.index'
        print("Building",file,index_dir)
        p = subprocess.Popen([executable_path,file,index_dir], stdout=subprocess.PIPE)
        out, err = p.communicate()
        yield out.decode(),index_dir
        # yield "out",index_dir

def find(*args, **kwargs):
    """ Find MEM tables """
    index = args[0]
    all = args[1]
    pattern = args[2]
    try:
        executable_path = get_exe("find_exe")
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

def minimize_files(*args, **kwargs):
    """ create minimized version of data, save entry with parameters on special output folder """
    files = args[0]
    m = str(args[1])
    w = str(args[2])

    executable_path = get_exe("minimize_exe")
    if not os.path.exists(executable_path) or  not os.access(executable_path, os.X_OK):
        raise Exception("Unable to find executable: {executable_path}")
        
    out_files = list()
    for file in files:
        out_file=file+f'_m{m}_w{w}'
        out_files.append(out_file)
        if not os.path.exists(out_file) or kwargs['rebuild']:
            try:
                if kwargs['line_parse']:
                    a = datetime.datetime.now()
                    with open(file,'r') as f:
                        for line in f.readlines():
                            p = subprocess.run([executable_path,m,w,line], stdout=open(out_file,'a'),stderr=subprocess.PIPE)
                    b = datetime.datetime.now()
                    c = b - a
                    print(f">Minimize_all_lines file:{file}\tm:{m}\tw:{w}\ttime:{c.microseconds}")
                else:
                    a = datetime.datetime.now()
                    p = subprocess.run([executable_path,m,w,file], stdout=open(out_file,'w'),stderr=subprocess.PIPE)
                    b = datetime.datetime.now()
                    c = b - a
                    print(f">Minimize m:{m}\tw:{w}\ttime:{c.microseconds}")
            except Exception as e:
                raise Exception(f'Error during minimization')

    return out_files

def get_exe(name):
    with open(cfg_file,'r') as f:
        kwargs = json.load(f)
    return kwargs["cpp_exe"][name]

def count_bytes(m,sigma=5):
    return math.ceil(math.log2(sigma**(m+1)-1)/6)

def get_compression_ratio(original,original_bits,compressed,compressed_bits):
    return round((len(compressed)*compressed_bits)/(len(original)*original_bits),5)

def get_alphabet(seq):
    return len(set(seq))

def minimize(*args, **kwargs):
    ''' For given list of files and lit of sequences minimize every file and seuqence with parameters m and window size w'''
    m = str(kwargs['m'])
    w = str(kwargs['w'])
    files = kwargs['files']
    sequences = kwargs['sequences']
    line_parse = kwargs['line_parse']
    rebuild = kwargs['rebuild']

    executable_path = get_exe("minimize_exe")
    if not os.path.exists(executable_path) or  not os.access(executable_path, os.X_OK):
        raise Exception("Unable to find executable: {executable_path}")
    
    out_files = list()
    try:
        for i,file in enumerate(files):
            out_file=file+f'_m{m}_w{w}'
            if not os.path.exists(out_file) or rebuild:
                a = datetime.datetime.now()
                if line_parse:
                    with open(file,'r') as f:
                        for line in f.readlines():
                            subprocess.run([executable_path,m,w,line], stdout=open(out_file,'a'),stderr=subprocess.PIPE)
                else:
                    subprocess.run([executable_path,m,w,file], stdout=open(out_file,'w'),stderr=subprocess.PIPE)
                b = datetime.datetime.now()
                print(f">Minimized file:{file}\tm:{m}\tw:{w}\ttime:{(b - a).microseconds}")
                print(out_file)
            out_files.append(out_file)

        for i,sequence in enumerate(sequences):
            a = datetime.datetime.now()
            p = subprocess.run([executable_path,m,w,sequence],capture_output = True, text = True)
            out = p.stdout
            b = datetime.datetime.now()
            sigma = get_alphabet(sequence)
            bytes = count_bytes(int(m),sigma)
            compression_ratio = get_compression_ratio(sequence,math.ceil(math.log2(sigma)),out,8)
            stat = sequence if len(sequence) < 50 else sequence[:50]+'...' 
            print(f'>Minimized sequence:{stat}\tm:{m}\tw:{w}\ttime:{(b - a).microseconds}c/mmer:{str(bytes)}\tcr:{compression_ratio}')
            print(out)

    except Exception as e:
        raise Exception(f'Error during minimization')
    
    return out_files

def minimize_sequences(*args, **kwargs):
    """ create minimized version of data, save entry with parameters on special output folder """
    sequences = args[0]
    m = str(args[1])
    w = str(args[2])

    executable_path = get_exe("minimize_exe")
    if not os.path.exists(executable_path) or  not os.access(executable_path, os.X_OK):
        raise Exception("Unable to find executable: {executable_path}")
        
    out_file = None
    for i,seq in enumerate(sequences):
        try:    
            p = subprocess.run([executable_path,m,w,seq],capture_output = True, text = True)
            out = p.stdout
            sigma = get_alphabet(seq)
            bytes = count_bytes(int(m),sigma)
            compression_ratio = get_compression_ratio(seq,math.ceil(math.log2(sigma)),out,8)
            stat = ''
            if len(seq) > 50:
                stat = seq[:50]+'...'
            else:
                stat = seq
            print(f'>{stat}\tm:{m}\tw:{w}\t c/mmer:{str(bytes)}\t cr:{compression_ratio}')
            print(out)
        except Exception as e:
            raise Exception(f'Error during minimization')
        
    return out_file

def kernelize_file(*args, **kwargs):
    """ create katka kernel of base data, save entry with parameters on special output folder """
    files = args[0]
    k = str(args[1])

    executable_path = get_exe("kernelize_exe")
    if not os.path.exists(executable_path) or  not os.access(executable_path, os.X_OK):
        raise Exception("Unable to find executable: {executable_path}")
        
    out_files = list()
    for file in files:
        out_file = file+f'_k{k}'
        out_files.append(out_file)
        if not os.path.exists(out_file):
            try:    
                a = datetime.datetime.now()
                p = subprocess.run([executable_path,k,file], stdout=open(out_file,'wb'),stderr=subprocess.PIPE)
                b = datetime.datetime.now()
                c = b - a
                print(f">Kernelize k:\tk:{k}\ttime:{c.microseconds}")
            except Exception as e:
                raise Exception(f'Error during kernelization') # problem with decoding minimization

    return out_files

def kernelize_sequence(*args, **kwargs):
    """ create katka kernel of base data, save entry with parameters on special output folder """
    sequences = args[0]
    k = str(args[1])

    executable_path = get_exe("kernelize_exe")
    if not os.path.exists(executable_path) or  not os.access(executable_path, os.X_OK):
        raise Exception("Unable to find executable: {executable_path}")
        
    out_files = list()
    for i,seq in enumerate(sequences):
        try:    
            p = subprocess.run([executable_path,k,seq],capture_output = True, text = True)
            out = p.stdout
            sigma = get_alphabet(seq)
            compression_ratio = get_compression_ratio(seq,1,out,1)
            print(f'>seq{i}\tk:{k}\t cr:{compression_ratio}')
            print(out)
        except Exception as e:
            raise Exception(f'Error during kernelization') # problem with decoding minimization

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

def get_base_file(*args,**kwargs):
    return kwargs["configure"]["base_file"]

def list_indices(*args, **kwargs):
    for index in indices():
        print(index)

def parse_inputs(inputs):
    files = list()
    sequences = list()
    for input in inputs:
        try:
            out = subprocess.run([f'find {input} -type f'], capture_output = True, text = True, shell=True).stdout
            if out == '':
                sequences.append(input)
            else:
                files += out.split('\n')[:-1]
        except Exception as e:
            raise Exception(f"Input parsing unsucessful due to {e}")
    return sequences,files

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
    parser_minimize.add_argument("-m", type=parse_range, help="kernel size (5 - 30), can be given range in format a-b-c as from a to b by c steps")
    parser_minimize.add_argument("-w", type=parse_range, help="minimizer window size (10 - 50), can be given range in format a-b-c as from a to b by c steps")
    parser_minimize.add_argument("inputs", nargs='+', type=str, help="input in format of set of sequences, set of files or folder that contains given original files")
    # parser_minimize.add_argument("-I", type=str, help="input path - accepts file or a folder. in case of folder minimize all available files in the folder")
    # parser_minimize.add_argument("-i", nargs='+',type=str, help="string to minimize")
    parser_minimize.add_argument("--line_parse", required=False, default=False, action='store_true', help="minimize every single line in the file individualy")
    parser_minimize.add_argument("--rebuild", required=False, default=False, action='store_true', help="if minimized file already exists, it will be replaced by the new one")

    # Kernelize command
    parser_kernelize = subparsers.add_parser("kernelize", help="From original file create its KATKA kernel")
    parser_kernelize.add_argument("-k", type=parse_range, help="k-mer length (5 - 500),can be given range in format a-b-c as from a to b by c steps")
    parser_kernelize.add_argument("-I", type=str, help="input path - accepts file or a folder. in case of folder minimize all available files in the folder")
    parser_kernelize.add_argument("-i", nargs='+',type=str, help="string to kernelize")
    # parser_minimize.add_argument("--line_parse", required=False, default=False, action='store_true', help="minimize every single line in the file individualy")
    

    # List command
    subparsers.add_parser("list", help="list all built indices")

    # Build command
    parser_build = subparsers.add_parser("build", help="Build the project")
    parser_build.add_argument("-m", type=parse_range, required = False, default = list(), help="kernel size (5 - 30), can be given range in format a-b-c as from a to b by c steps")
    parser_build.add_argument("-w", type=parse_range, required = False, default = list(), help="minimizer window size (10 - 50), can be given range in format a-b-c as from a to b by c steps")
    parser_build.add_argument("-k", type=parse_range, required = False, default = list(), help="k-mer length (5 - 500),can be given range in format a-b-c as from a to b by c steps")
    parser_build.add_argument("-I", type=str, required=True , help="path to the file")
    parser_build.add_argument("--rebuild", required=False, default=False, action='store_true', help="if minimized file already exists, it will be replaced by the new one")

    # find command
    parser_find = subparsers.add_parser("find_leaf", help="For every pattern returns the best leaf match (longest singleton MEM)")
    parser_find.add_argument("index", type=str, help="path to the index directory")
    parser_find.add_argument("patterns", nargs='+', type=str, help="Single pattern or a pattern file (one pattern per line)")

    # findall command
    find_mems = subparsers.add_parser("find_mems", help="find MEMs of given patterns with respect to the tree data")
    find_mems.add_argument("index", type=str, help="path to the index directory")
    find_mems.add_argument("patterns", nargs='+', type=str, help="Single pattern or a pattern file (one pattern per line)")

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
        if args.i:
            #   minimize sequence given on input
            for k in args.k:
                kernelize_sequence(args.i,k)
        elif args.I:
            if not os.path.exists(args.I):
                raise FileNotFoundError(f"Given input wasnt found on {args.I}")
            files = [args.I]
            for k in args.k:
                out_files = kernelize_file(files,k)   
                print(out_files)
        else:
            files = [get_base_file()]
            for k in args.k:
                out_files = kernelize_file(files,k)   
                print(out_files)
    elif args.command == "find_mems":
        for pattern in patterns(args.patterns):
            out = find(args.index,1,pattern)
            print(out)
    elif args.command == "find_leaf":
        for pattern in patterns(args.patterns):
            out = find(args.index,0,pattern)
            print(out) 

    elif args.command == "minimize":
        sequences,files = parse_inputs(args.inputs)
        #   minimize sequence given on input
        for m in args.m:
            for w in args.w:
                if m<w:
                    out_files = minimize(files = files, sequences = sequences,m=m,w=w,line_parse=args.line_parse, rebuild=args.rebuild)
 
    elif args.command == "build":
        files=[args.I]
        min_files = list()
        for m in args.m:
            for w in args.w:
                if m<w:
                    min_files += minimize_files(files,m,w,rebuild=args.rebuild)        

        if min_files != list():
            files = min_files

        ker_files = list()
        for k in args.k:
            ker_files += kernelize_file(files,k,rebuild=args.rebuild)

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
