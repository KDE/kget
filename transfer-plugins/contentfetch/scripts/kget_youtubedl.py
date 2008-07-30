#!/usr/bin/env python
#
# Copyright (c) 2006-2008 Ricardo Garcia Gonzalez
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
# 
# Except as contained in this notice, the name(s) of the above copyright
# holders shall not be used in advertising or otherwise to promote the
# sale, use or other dealings in this Software without prior written
# authorization.
#
import getpass
import os
import re
import string
import sys
import urllib
import urllib2

# Global constants
const_video_url_str = 'http://www.youtube.com/watch?v=%s'
const_video_url_re = re.compile(r'^((?:http://)?(?:\w+\.)?youtube\.com/(?:(?:v/)|(?:(?:watch(?:\.php)?)?\?(?:.+&)?v=)))?([0-9A-Za-z_-]+)(?(1).+)?$')
const_video_url_format_suffix = '&fmt=%s'
const_best_quality_format = 18
const_login_url_str = 'http://www.youtube.com/login?next=/watch%%3Fv%%3D%s'
const_login_post_str = 'current_form=loginForm&next=%%2Fwatch%%3Fv%%3D%s&username=%s&password=%s&action_login=Log+In'
const_bad_login_re = re.compile(r'(?i)<form[^>]* name="loginForm"')
const_age_url_str = 'http://www.youtube.com/verify_age?next_url=/watch%%3Fv%%3D%s'
const_age_post_str = 'next_url=%%2Fwatch%%3Fv%%3D%s&action_confirm=Confirm'
const_url_t_param_re = re.compile(r', "t": "([^"]+)"')
const_video_url_real_str = 'http://www.youtube.com/get_video?video_id=%s&t=%s'
const_video_title_re = re.compile(r'<title>YouTube - ([^<]*)</title>', re.M | re.I)


# Wrapper to create custom requests with typical headers
def request_create(url, extra_headers, post_data):
	retval = urllib2.Request(url)
	if post_data is not None:
		retval.add_data(post_data)
	# Try to mimic Firefox, at least a little bit
	retval.add_header('User-Agent', 'Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.1.14) Gecko/20080404 Firefox/2.0.0.14')
	retval.add_header('Accept-Charset', 'ISO-8859-1,utf-8;q=0.7,*;q=0.7')
	retval.add_header('Accept', 'text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5')
	retval.add_header('Accept-Language', 'en-us,en;q=0.5')
	if extra_headers is not None:
		for header in extra_headers:
			retval.add_header(header[0], header[1])
	return retval

# Perform a request, process headers and return response
def perform_request(url, headers=None, data=None):
	request = request_create(url, headers, data)
	response = urllib2.urlopen(request)
	return response

# Generic download step
def download_step(return_data_flag, url, post_data=None):
	data = perform_request(url, data=post_data).read()
	if return_data_flag:
		return data
	return None

# Generic extract step
def extract_step(regexp, data):
	match = regexp.search(data)
	extracted_data = match.group(1)
	return extracted_data

def startDownload(kconfig):
	import kgetcore
	# Verify video URL format and convert to "standard" format
	video_url_cmdl = kgetcore.getSourceUrl()
        #video_url_cmdl = 'http://www.youtube.com/watch?v=k6mEirkQN8o&feature=dir'
	video_url_mo = const_video_url_re.match(video_url_cmdl)
	if video_url_mo is None:
		video_url_mo = const_video_url_re.match(urllib.unquote(video_url_cmdl))
	video_url_id = video_url_mo.group(2)
	video_url = const_video_url_str % video_url_id, video_url_id
	video_url = video_url[0]
	
	video_format = None

	#if video_format is not None:
	#	video_url = [('%s%s' % (x, const_video_url_format_suffix % video_format), y) for (x, y) in video_urls]

        # Install cookie and proxy handlers
	urllib2.install_opener(urllib2.build_opener(urllib2.ProxyHandler()))
	urllib2.install_opener(urllib2.build_opener(urllib2.HTTPCookieProcessor()))

        # Download all the given videos
	print video_url
        # Retrieve video webpage
	video_webpage = download_step(True, video_url)

        # Extract video title if needed
        # video_title = extract_step(const_video_title_re, video_webpage)

        # Extract needed video URL parameters
	video_url_t_param = extract_step(const_url_t_param_re, video_webpage)
	video_url_real = const_video_url_real_str % (video_url_id, video_url_t_param)
	if video_format is not None:
		video_url_real = '%s%s' % (video_url_real, const_video_url_format_suffix % video_format)
	kgetcore.addTransfer(video_url_real)
        #print video_url_real
