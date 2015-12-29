#ifdef __DARWIN__
#include <sys/types.h>
#endif

#include <pthread.h>
#include "communityMQTT_common.h"

void releasePayload(Payload * pPayload);

pthread_mutex_t payload_mutex = PTHREAD_MUTEX_INITIALIZER;

bool pushPayload(SessionHandle * pSession, Payload * pPayload)
{
  if (!pSession || !pPayload)
    return false;
  
  bool result = true;
  pthread_mutex_lock(&payload_mutex);
  Payload * pHead = pSession->pHead;
  if (!pHead)
  {
    pHead = pPayload;
    pHead->pNext = NULL;
    pSession->pHead = pHead;
  }
  else
  {
    Payload * pTemp = pHead;
    while (pTemp->pNext)
    {
      pTemp = pTemp->pNext;
      if (pTemp == pHead)
      {
        result = false;
        break;
      }
    }
    if (result)
    {
      pTemp->pNext = pPayload;
      pPayload->pNext = NULL;
    }
  }
  pthread_mutex_unlock(&payload_mutex);
  return result;
}

Payload * curPayload(SessionHandle * pSession)
{
    if (pSession)
        return pSession->pHead;
    else
        return NULL;
}

bool popPayload(SessionHandle * pSession)
{
  if (!pSession)
    return false;

  bool result = true;
  pthread_mutex_lock(&payload_mutex);
  Payload * pHead = pSession->pHead;
  if (!pHead) 
    result = false;
  else 
  {
    Payload * pTemp = pHead;
    pHead = pHead->pNext;
    releasePayload(pTemp);
    pTemp = NULL;
    pSession->pHead = pHead;
  }
  pthread_mutex_unlock(&payload_mutex);
  return result;
}

void releasePayload(Payload * pPayload)
{
  if (!pPayload) 
    return;
  
  switch (pPayload->type)
  {
    case StartSessionTask:
      {
        StartSessionData * pData = pPayload->pStartSessionData;
        free(pData->host);
        free(pData->clientid);
        free(pData->username);
        free(pData->password);
        free(pData);
        break;
      }
    case PublishTask:
      {
        PublishData * pData = pPayload->pPublishData;
        free(pData->topic);
        free(pData->payload);
        free(pData);
        break;
      }
    case SubscribeTask:
      {
        SubscribeData * pData = pPayload->pSubscribeData;
        free(pData->topic);
        free(pData);
        break;
      }
    case UnsubscribeTask:
      {
        UnsubscribeData * pData = pPayload->pUnsubscribeData;
        free(pData->topic);
        free(pData);
        break;
      }
    default:
      break;
  }
  free(pPayload);
}
