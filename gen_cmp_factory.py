import os, glob, re

path = os.getcwd() + "/src/cmp"

out = open("./src/cmp_factory.h", "w");

# NOTE: include components files --------------------------------------------------

for filename in glob.glob(os.path.join(path, '*.h')) :
    out.write("#include \"cmp/" + os.path.basename(filename) + "\"\n")

out.write("#include \"tokenizer.h\"")



# NOTE: Write Start AddTypesToEntityManager function ------------------------------

out.write("\n\nstatic void AddTypesToEntityManager() {\n")

# NOTE: Write compoenets if statement ---------------------------------------------

for filename in glob.glob(os.path.join(path, '*.h')) :

    file = open(filename)
    content = file.read()

    component_type = re.findall("struct\\s+.+:\\s*CMP<(.+)>", content)
    
    if len(component_type) >= 1 :
        out.write("    EntityManager::Get()->AddComponentType<" + component_type[0] + ">();\n")

# NOTE: Write end function ------------------------------------------------------

out.write("}")




# NOTE: Write Start CreateCMPFromNextToken function -------------------------------

out.write("\n\nstatic CMPBase *CreateCMPFromNextToken(Tokenizer *t, Entity_ *entity) {\n")
out.write("    Tokenizer tmp = *t;\n")
out.write("    Token token;\n")
out.write("    tmp.NextToken(&token);\n")

# NOTE: Write compoenets if statement ---------------------------------------------

first = True 

for filename in glob.glob(os.path.join(path, '*.h')) :

    file = open(filename)
    content = file.read()

    component_type = re.findall("struct\\s+.+:\\s*CMP<(.+)>", content)
    component_name = re.findall("ReadBeginObject\\(t,\\s*\"(.*)\"\\s*\\);", content)
    
    if_statement = "else if"

    if len(component_type) >= 1 and len(component_name) >= 1 :
        if first :
            if_statement = "if"
            first = False
    
        out.write("    " + if_statement + "(token.Contains(\"" + component_name[0] +"\")) {\n")
        out.write("        return EntityManager::Get()->AddComponent<" + component_type[0] + ">(entity->key);\n")
        out.write("    }\n")

# NOTE: Write end function ------------------------------------------------------

out.write("    ASSERT(!\"cmp factory invalid code path\");\n")
out.write("    return nullptr;\n")
out.write("}")

