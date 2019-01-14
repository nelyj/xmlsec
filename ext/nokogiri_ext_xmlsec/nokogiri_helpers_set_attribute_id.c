#include "xmlsecrb.h"

VALUE set_id_attribute(VALUE self, VALUE rb_attr_name) {
  xmlDocPtr doc;
  xmlNodePtr node;
  xmlAttrPtr attr;
  xmlAttrPtr tmp;
  xmlChar *name;
  const xmlChar *idName;

  Data_Get_Struct(self, xmlDoc, doc);
  Data_Get_Struct(self, xmlNode, node);
  Check_Type(rb_attr_name, T_STRING);
  idName = (const xmlChar *)RSTRING_PTR(rb_attr_name);
  xmlXPathContextPtr xpathCtx = xmlXPathNewContext(doc);

  xmlChar* xpathExpr = "//*[@ID | @Id | @id]";

  xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);

  if(xpathObj == NULL) {
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(doc); 
    rb_raise(rb_eRuntimeError,"Error: unable to evaluate xpath expression \"%s\"\n", xpathExpr);
    return(-1);
  }

  xmlNodeSetPtr nodes = xpathObj->nodesetval;
  int size = (nodes) ? nodes->nodeNr : 0;
  char* idNames[] = {"ID", "Id", "id"};
  printf("Nodes found: %i \n", size);
  printf("rb_attr_name: %s \n", idName);

  xmlAttrPtr attr, tmp;
  int i,j;
  for(i = 0; i < size; i++) {
    for(j=0; j<3;j++) {
      tmp = xmlHasProp(nodes->nodeTab[i], idNames[j]);
      if(tmp != NULL) {
        attr = tmp;
      }
    }
    if(attr == NULL) {
      xmlXPathFreeContext(xpathCtx); 
      return(-1);
    }
    xmlChar* name = xmlNodeListGetString(doc, attr->children, 1);
    printf("name: %s \n", name);
    if(name == NULL) {
      xmlXPathFreeContext(xpathCtx); 
      return(-1);
    }
    xmlAttrPtr tmp = xmlGetID(doc, name);
    if(tmp != NULL) {
      xmlFree(name);
      return 0;
    }
    xmlAddID(NULL, doc, name, attr);
    xmlFree(name);
  }
  // find pointer to id attribute
  //attr = xmlHasProp(node, idName);
  //if((attr == NULL) || (attr->children == NULL)) {
  //  rb_raise(rb_eRuntimeError, "Can't find attribute to add register as id");
  //  return Qfalse;
  //}
  
  // get the attribute (id) value
  //name = xmlNodeListGetString(node->doc, attr->children, 1);
  //if(name == NULL) {
  //  rb_raise(rb_eRuntimeError, "Attribute %s has no value", idName);
  //  return Qfalse;
  //}
  
  // check that we don't have that id already registered
  //tmp = xmlGetID(node->doc, name);
  //if(tmp != NULL) {
    // rb_raise(rb_eRuntimeError, "Attribute %s is already an ID", idName);
  //  xmlFree(name);
  //  return Qfalse;
  //}
  
  // finally register id
  //xmlAddID(NULL, node->doc, name, attr);

  // and do not forget to cleanup
  //xmlFree(name);

  return Qtrue;
}
