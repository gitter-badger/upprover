#include "constants.h"

static u_int
encode_ie(void *buf, size_t bufsize,                  // 8-byte character array
               const u_int8_t *ie, size_t ielen,          // 8-byte uint array
	       const char *leader, size_t leader_len)
{
  /* buf is treated as an array of unsigned 8-byte ints */
  u_int8_t *p;
  int i;

  // copy the contents of leader into buf
  if (bufsize < leader_len)
    return 0;
  p = buf;
  memcpy(p, leader, leader_len);
  bufsize -= leader_len;
  p += leader_len;

  /* This is the fix. */
  if (bufsize < ielen)
    return 0;

  for (i = 0; i < ielen && bufsize > 2; i++) {
    /* This was originally
     *    p += sprintf(p, "%02x", ie[i]);
     * This would print two digits from ie[i] into p, and 
     * return the number of bytes written.
     *
     * Simplified to remove sprintf.
     *
     */
    /* OK */
    *p = 'x';
    /* OK. */
__TESTCLAIM_1:
    *(p+1) = 'x';
    p += 2;
  }

  // if we wrote all of ie[], say how many bytes written in total, 
  // otherwise, claim we wrote nothing
  return (i == ielen ? p - (u_int8_t *)buf : 0);
}


static int
giwscan_cb(const struct ieee80211_scan_entry *se)
{
  u_int8_t buf[BUFSZ];
  char rsn_leader [LEADERSZ];

  /* Everything up to this point seems irrelevant to the following. */

  if (se->se_rsn_ie != NULL) {
    if (se->se_rsn_ie[0] == IEEE80211_ELEMID_RSN)
      encode_ie(buf, sizeof(buf),
                se->se_rsn_ie, se->se_rsn_ie[1] + 2,
                rsn_leader, sizeof(rsn_leader) - 1);
  }
  
  return 0;
}

int main ()
{
  struct ieee80211_scan_entry se;
  u_int8_t ie [IESZ];
  se.se_rsn_ie = ie;
  se.se_rsn_ie[1] = IESZ - 2;

  int res = giwscan_cb (&se);
  assert (res == 0);
  return 0;
}
