#include "xmlsecrb.h"

VALUE sign_with_certificate(VALUE self, VALUE rb_key_name, VALUE rb_rsa_key, VALUE rb_cert, VALUE rb_uri) {
  xmlDocPtr doc;
  xmlNodePtr signNode = NULL;
  xmlNodePtr refNode = NULL;
  xmlNodePtr keyInfoNode = NULL;
  xmlSecDSigCtxPtr dsigCtx = NULL;
  char uriVar[50] = "";
  char *keyName;
  char *certificate;
  char *rsaKey;
  char *idXml;
  unsigned int rsaKeyLength, certificateLength;


  Data_Get_Struct(self, xmlDoc, doc);
  rsaKey = RSTRING_PTR(rb_rsa_key);
  rsaKeyLength = RSTRING_LEN(rb_rsa_key);
  idXml = RSTRING_PTR(rb_uri);
  keyName = RSTRING_PTR(rb_key_name);
  certificate = RSTRING_PTR(rb_cert);
  certificateLength = RSTRING_LEN(rb_cert);

  strcat(uriVar, "#");
  strcat(uriVar, idXml);

  printf("ID rb var: %s \n", idXml);
  printf("URI rb var: %s \n", uriVar);

  // create signature template for RSA-SHA1 enveloped signature
  signNode = xmlSecTmplSignatureCreate(doc, xmlSecTransformInclC14NId,
                                         xmlSecTransformRsaSha1Id, NULL);
  if (signNode == NULL) {
    rb_raise(rb_eSigningError, "failed to create signature template");
    goto done;
  }

  // add <dsig:Signature/> node to the doc
  xmlAddChild(xmlDocGetRootElement(doc), signNode);

  //add reference
  if(strcmp(uriVar,"#") == 0) {
    refNode = xmlSecTmplSignatureAddReference(signNode, xmlSecTransformSha1Id, NULL, NULL, NULL);
  } else {
    //refNode = xmlSecTmplSignatureAddReference(signNode, xmlSecTransformSha1Id, idXml, (xmlChar*)uriVar, NULL);
    refNode = xmlSecTmplSignatureAddReference(signNode, xmlSecTransformSha1Id, NULL, (xmlChar*)uriVar, NULL);
  }

  if(refNode == NULL) {
    rb_raise(rb_eSigningError, "failed to add reference to signature template");
    goto done;
  }

  // add enveloped transform
  if(xmlSecTmplReferenceAddTransform(refNode, xmlSecTransformEnvelopedId) == NULL) {
    rb_raise(rb_eSigningError, "failed to add enveloped transform to reference");
    goto done;
  }

  // add <dsig:KeyInfo/> and <dsig:X509Data/>
  keyInfoNode = xmlSecTmplSignatureEnsureKeyInfo(signNode, NULL);
  if(keyInfoNode == NULL) {
    rb_raise(rb_eSigningError, "failed to add key info");
    goto done;
  }

  if(xmlSecTmplKeyInfoAddKeyValue(keyInfoNode) == NULL) {
    rb_raise(rb_eSigningError, "failed to add key info");
    goto done;
  }

  if(xmlSecTmplKeyInfoAddX509Data(keyInfoNode) == NULL) {
    rb_raise(rb_eSigningError, "failed to add X509Data node");
    goto done;
  }

  // create signature context, we don't need keys manager in this example
  dsigCtx = xmlSecDSigCtxCreate(NULL);
  if(dsigCtx == NULL) {
    rb_raise(rb_eSigningError, "failed to create signature context");
    goto done;
  }

  // load private key, assuming that there is not password
  dsigCtx->signKey = xmlSecCryptoAppKeyLoadMemory((xmlSecByte *)rsaKey,
                                                  rsaKeyLength,
                                                  xmlSecKeyDataFormatPem,
                                                  NULL, // password
                                                  NULL,
                                                  NULL);
  if(dsigCtx->signKey == NULL) {
    rb_raise(rb_eSigningError, "failed to load private key");
    goto done;
  }
  
  // load certificate and add to the key
  if(xmlSecCryptoAppKeyCertLoadMemory(dsigCtx->signKey,
                                      (xmlSecByte *)certificate,
                                      certificateLength,
                                      xmlSecKeyDataFormatPem) < 0) {
    rb_raise(rb_eSigningError, "failed to load certificate");
    goto done;
  }

  // set key name
  if(xmlSecKeySetName(dsigCtx->signKey, (xmlSecByte *)keyName) < 0) {
    rb_raise(rb_eSigningError, "failed to set key name");
    goto done;
  }

  // sign the template
  if(xmlSecDSigCtxSign(dsigCtx, signNode) < 0) {
    rb_raise(rb_eSigningError, "signature failed");
    goto done;
  }

done:
  if(dsigCtx != NULL) {
    xmlSecDSigCtxDestroy(dsigCtx);
  }

  return T_NIL;
}
