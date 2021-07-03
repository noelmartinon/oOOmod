/*
    GPLv3 license
    Copyleft 2021 - Noël Martinon

    THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY
    APPLICABLE LAW.  EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT
    HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY
    OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM
    IS WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF
    ALL NECESSARY SERVICING, REPAIR OR CORRECTION.
*/

#include "pugixml.hpp"
#include "common.hpp"
#include <sstream>

int g_argc;
char **g_argv;

//-----------------------------------------------
std::string Generate_XPath(pugi::xml_node &node, bool bwithtext = true) {
    if (std::string(node.name()) == "") return "";
    std::string xpath = std::string("/") + node.name();

    std::string attributes;
    for (pugi::xml_attribute attr = node.first_attribute(); attr;)
    {
        if (attributes.empty()) attributes = "[";
        else attributes += " and ";

        pugi::xml_attribute nextAttr = attr.next_attribute();
        attributes += std::string("@") + attr.name() + std::string("=\"") + attr.value() + std::string("\"");
        attr = nextAttr;
    }
    if (!attributes.empty()) attributes += "]";
    xpath += attributes;

    if (bwithtext && !std::string(node.text().get()).empty())
        xpath += "[text()=\""+std::string(node.text().get())+"\"]";

    for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling())
        xpath += Generate_XPath(child, bwithtext);

    return xpath;
}
//-----------------------------------------------
bool SetXMLValue(pugi::xml_document &doc, std::string oor_path, std::string oor_name, std::string oor_value) {
    std::string path = "/oor:items/item[@oor:path='"+oor_path+"']/prop[@oor:name='"+oor_name+"']"+"/value[text()='"+oor_value+"']";

    // Check if node with value already exists
    pugi::xpath_node xpathnode = doc.select_node(path.c_str());
    if (xpathnode) return false;

    // Modify existing node or create it
    path = "/oor:items/item[@oor:path='"+oor_path+"']/prop[@oor:name='"+oor_name+"']";
    xpathnode = doc.select_node(path.c_str());
    if (xpathnode) {
        pugi::xml_node node = xpathnode.node().child("value");
        if (!node)
            node = xpathnode.node().append_child("value");
        node.text().set(oor_value.c_str());
    }
    else {
        pugi::xml_node nodeBase = doc.child("oor:items");

        pugi::xml_node nodeItem = nodeBase.append_child("item");
        nodeItem.append_attribute("oor:path") = oor_path.c_str();

        pugi::xml_node nodeChild = nodeItem.append_child("prop");
        nodeChild.append_attribute("oor:name") = oor_name.c_str();
        nodeChild.append_attribute("oor:op") = "fuse";
        nodeChild.append_child("value").text().set(oor_value.c_str());
    }

    return true;
}
//-----------------------------------------------
/**
 *  AppendXMLString()
 *  Proceed registrymodifications.xuc file
 */
bool AppendXMLString(pugi::xml_node target, const std::string& srcString)
{
  // parse XML string as document
  pugi::xml_document doc;
  if (!doc.load_buffer(srcString.c_str(), srcString.length()))
    return false;

  for (pugi::xml_node child = doc.first_child(); child; child = child.next_sibling())
    target.append_copy(child);
  return true;
}
//-----------------------------------------------
/**
 *  GetXMLconfig()
 *  Read xml config file
 */
bool GetXMLconfig(std::string filename) {
    std::cout << "Reading " << filename << std::endl;

    pugi::xml_document doc;
    if (!doc.load_file(filename.c_str())) return false;

    // A valid XML document must have a single root node (only one !)
    pugi::xml_node root = doc.document_element();

    for (pugi::xml_node node = root.first_child(); node; )
    {
        pugi::xml_node next = node.next_sibling();
        node.print(std::cout, "", pugi::format_raw);
        std::cout << "\n";
        for (pugi::xml_attribute attr = node.child("prop").first_attribute(); attr; attr = attr.next_attribute())
        {
            std::cout << " " << attr.name() << "=" << attr.value() << std::endl;
        }
        node = next;
    }

    return true;
}
//-----------------------------------------------
/**
 *  Process_xcu()
 *  Proceed registrymodifications.xuc file
 */
bool Process_xcu(std::string filename) {
    std::cout << "Processing " << filename << std::endl;

    bool is_modified = false;
    pugi::xml_document doc;
    if (!doc.load_file(filename.c_str())) return false;
    pugi::xml_node root = doc.document_element();

    // ***** Import XML config nodes if required *****
    //
    if (g_argc > 1) {
        std::cout << "Reading " << g_argv[1] << std::endl;
        pugi::xml_document config;
        if (!config.load_file(g_argv[1])) return false;

        // A valid XML document must have a single root node
        pugi::xml_node root_config = config.document_element();
        for (pugi::xml_node node = root_config.first_child(); node; node = node.next_sibling()) {
            std::string xpath = "/" + std::string(node.parent().name()) + Generate_XPath(node);

            // Check if node with value already exists
            pugi::xpath_node xpathnode_doc = doc.select_node(xpath.c_str());
            if (xpathnode_doc) continue;
            is_modified = true;

            // Generate XPath from config node
            xpath = "/" + std::string(node.parent().name()) + Generate_XPath(node, false);
            xpathnode_doc = doc.select_node(xpath.c_str());
            pugi::xpath_node xpathnode_config = config.select_node(xpath.c_str());

            // If node already exists, set his value
            if (xpathnode_doc) {
                std::cout << "UPDATE " << xpath << std::endl;
                pugi::xml_node selectedNode = xpathnode_doc.node();
                std::cout << "-> Replace value \"" << selectedNode.text().get() << "\" with \"" << xpathnode_config.node().text().get() << "\"" << std::endl;
                selectedNode.text().set(xpathnode_config.node().text().get());
            }
            // else add node
            else {
                std::cout << "ADD " << xpath << std::endl;
                std::cout << "-> Set value \"" << xpathnode_config.node().text().get() << "\"" << std::endl;
                std::stringstream ss;
                node.print(ss, "");
                std::string path = ss.str();
                AppendXMLString(root, path);
            }
        }
    }

    // ***** Set identity (Windows)*****
    //
    bool setIdentity=false;

    // ================
    #ifdef _WIN32

    std::string oor_path = "";
    std::string oor_name = "";
    std::string oor_value = "";

    std::string username = "";
    std::string fullname = "";
    std::string domain = "";
    std::string firstname = "";
    std::string surname = "";
    std::string initials = "";

    if (GetUserName(username)) {
        if (GetWorkgroup(domain)) {
            if (GetFullName(username, domain, fullname)) {
                setIdentity=true;
                std::string::size_type pos=fullname.find(' ',0);
                if (pos == std::string::npos)
                    setIdentity=false;
                else {
                    firstname=fullname.substr(0,pos);
                    surname=fullname.substr(pos+1);
                    initials += firstname.at(0);
                    initials += surname.at(0);
                }
            }
        }
    }

    // ***** Set firstname *****
    // <item oor:path="/org.openoffice.UserProfile/Data"><prop oor:name="givenname" oor:op="fuse"><value>__firstname__</value></prop></item>
    //
    if (setIdentity && !firstname.empty()) {
        oor_path = "/org.openoffice.UserProfile/Data";
        oor_name = "givenname";
        oor_value = firstname;
        if (SetXMLValue(doc, oor_path, oor_name, oor_value))
            is_modified = true;
    }

    // ***** Set surname *****
    // <item oor:path="/org.openoffice.UserProfile/Data"><prop oor:name="sn" oor:op="fuse"><value>__surname__</value></prop></item>
    //
    if (setIdentity && !surname.empty()) {
        oor_path = "/org.openoffice.UserProfile/Data";
        oor_name = "sn";
        oor_value = surname;
        if (SetXMLValue(doc, oor_path, oor_name, oor_value))
            is_modified = true;
    }

    // ***** Set initials *****
    // <item oor:path="/org.openoffice.UserProfile/Data"><prop oor:name="initials" oor:op="fuse"><value>__initials__</value></prop></item>
    if (setIdentity && !initials.empty()) {
        oor_path = "/org.openoffice.UserProfile/Data";
        oor_name = "initials";
        oor_value = initials;
        if (SetXMLValue(doc, oor_path, oor_name, oor_value))
            is_modified = true;
    }

    #endif
    // ================

    // Save xcu if it was modified
    if (is_modified) {
        std::cout << "Updating " << filename << std::endl;
        doc.save_file(filename.c_str());
    }

    return is_modified;
}
//-----------------------------------------------
/**
 *  Process_xcu_test()
 *  Proceed registrymodifications.xuc file
 */
bool Process_xcu_old(std::string filename) {
    std::cout << "Processing " << filename << std::endl;

    bool is_modified = false;
    pugi::xml_document doc;
    if (!doc.load_file(filename.c_str())) return false;

    //~ pugi::xml_node root = doc.document_element();
    //~ AppendXMLString(root, "<item oor:path=\"/org.openoffice.Setup/L10N\"><prop oor:name=\"ooLocale\" oor:op=\"fuse\"><value>fr</value></prop></item>");
    //~ doc.save_file("output.xcu");
    //~ return true;

    // ***** Set UI-lang fr *****
    // <item oor:path="/org.openoffice.Setup/L10N"><prop oor:name="ooLocale" oor:op="fuse"><value>fr</value></prop></item>
    //
    std::string oor_path = "/org.openoffice.Setup/L10N";
    std::string oor_name = "ooLocale";
    std::string oor_value = "fr";
    if (SetXMLValue(doc, oor_path, oor_name, oor_value))
        is_modified = true;

    // ***** Set locale system lang fr *****
    // <item oor:path="/org.openoffice.Setup/L10N"><prop oor:name="ooSetupSystemLocale" oor:op="fuse"><value>fr</value></prop></item>
    //
    oor_path = "/org.openoffice.Setup/L10N";
    oor_name = "ooSetupSystemLocale";
    oor_value = "fr";
    if (SetXMLValue(doc, oor_path, oor_name, oor_value))
        is_modified = true;

    // ***** Disable Load printer setting *****
    // <item oor:path="/org.openoffice.Office.Common/Save/Document"><prop oor:name="LoadPrinter" oor:op="fuse"><value>false</value></prop></item>
    //
    oor_path = "/org.openoffice.Office.Common/Save/Document";
    oor_name = "LoadPrinter";
    oor_value = "false";
    if (SetXMLValue(doc, oor_path, oor_name, oor_value))
        is_modified = true;

    // ***** Set identity *****
    //
    bool setIdentity=false;

    // ================
    #ifdef _WIN32

    std::string username = "";
    std::string fullname = "";
    std::string domain = "";
    std::string firstname = "";
    std::string surname = "";
    std::string initials = "";

    if (GetUserName(username)) {
        if (GetWorkgroup(domain)) {
            if (GetFullName(username, domain, fullname)) {
                setIdentity=true;
                std::string::size_type pos=fullname.find(' ',0);
                if (pos == std::string::npos)
                    setIdentity=false;
                else {
                    firstname=fullname.substr(0,pos);
                    surname=fullname.substr(pos+1);
                    initials += firstname.at(0);
                    initials += surname.at(0);
                }
            }
        }
    }

    //~ std::cout << username << std::endl;
    //~ std::cout << domain << std::endl;
    //~ std::cout << fullname << std::endl;
    //~ std::cout << firstname << std::endl;
    //~ std::cout << surname << std::endl;
    //~ std::cout << initials << std::endl;

    // ***** Set firstname *****
    // <item oor:path="/org.openoffice.UserProfile/Data"><prop oor:name="givenname" oor:op="fuse"><value>__firstname__</value></prop></item>
    //
    if (setIdentity && !firstname.empty()) {
        oor_path = "/org.openoffice.UserProfile/Data";
        oor_name = "givenname";
        oor_value = firstname;
        if (SetXMLValue(doc, oor_path, oor_name, oor_value))
            is_modified = true;
    }

    // ***** Set surname *****
    // <item oor:path="/org.openoffice.UserProfile/Data"><prop oor:name="sn" oor:op="fuse"><value>__surname__</value></prop></item>
    //
    if (setIdentity && !surname.empty()) {
        oor_path = "/org.openoffice.UserProfile/Data";
        oor_name = "sn";
        oor_value = surname;
        if (SetXMLValue(doc, oor_path, oor_name, oor_value))
            is_modified = true;
    }

    // ***** Set initials *****
    // <item oor:path="/org.openoffice.UserProfile/Data"><prop oor:name="initials" oor:op="fuse"><value>__initials__</value></prop></item>
    if (setIdentity && !initials.empty()) {
        oor_path = "/org.openoffice.UserProfile/Data";
        oor_name = "initials";
        oor_value = initials;
        if (SetXMLValue(doc, oor_path, oor_name, oor_value))
            is_modified = true;
    }

    #endif
    // ================

    // ***** Set organisation *****
    // <item oor:path="/org.openoffice.UserProfile/Data"><prop oor:name="o" oor:op="fuse"><value>__organisation__</value></prop></item>
    //
    std::string organisation = "";
    if (g_argc > 1) organisation = g_argv[1];
    if (!organisation.empty()) {
        oor_path = "/org.openoffice.UserProfile/Data";
        oor_name = "o";
        oor_value = organisation;
        if (SetXMLValue(doc, oor_path, oor_name, oor_value))
            is_modified = true;
    }

    // Save xcu if it was modified
    if (is_modified)
        doc.save_file(filename.c_str());

    return is_modified;
}

//-----------------------------------------------

