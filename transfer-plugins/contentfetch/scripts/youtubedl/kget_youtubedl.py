#!/usr/bin/env python
#
# This file is part of the KDE project
#
#  Copyright (C) 2008 Ningyu Shi <shiningyu@gmail.com>

#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public
#  License as published by the Free Software Foundation; either
#  version 2 of the License, or (at your option) any later version.

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

global_error = False
# Print error message, followed by standard advice information, and then exit
def exit_with_error(error_text):
    global global_error
    import kgetcore
    kgetcore.abort('Error: ' + error_text)
    global_error = True

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

# Conditional print
def cond_print(str):
    import kgetcore
    kgetcore.setTextStatus(str)

# Title string normalization
def title_string_norm(title):
    title = ''.join((x in string.ascii_letters or x in string.digits) and x or ' ' for x in title)
    title = '_'.join(title.split())
    title = title.lower()
    return title

# Generic download step
def download_step(return_data_flag, step_title, step_error, url, post_data=None):
    try:
        cond_print('%s... ' % step_title)
        data = perform_request(url, data=post_data).read()
        cond_print('%s... Done.' % step_title)
        if return_data_flag:
            return data
        return None
    except (urllib2.URLError, ValueError, httplib.HTTPException, TypeError, socket.error):
        cond_print('%s... Failed.' % step_title )
        exit_with_error(step_error)


# Generic extract step
def extract_step(step_title, step_error, regexp, data):
    cond_print('%s... ' % step_title)
    match = regexp.search(data)

    if match is None:
        cond_print('%s... Failed.' % step_title )
        exit_with_error(step_error)

    extracted_data = match.group(1)
    cond_print('%s... Done.' % step_title)
    return extracted_data


class YoutubeDlOption:
    def __init__(self, kconfig):
        self.parser = kconfig
        self.parser.setFile('kget_youtubedl.rc')
    def parse(self):
        self.use_literal = bool(self.parser.read('FileSetting', 'UseLiteralName', 0))
        if self.parser.read("FileSetting", "Quality", 1) == 0:
            # index 0 of the combobox is best quality
            self.best_quality = True
        else:
            self.best_quality = False
        self.enable_login = bool(self.parser.read('LoginInfo', 'Enabled', 0))
        self.username = self.parser.read('LoginInfo', 'Username', 'Nobody')
        self.password = self.parser.read('LoginInfo', 'Password', 'passwd')
        self.use_netrc = bool(self.parser.read('LoginInfo', 'UseNetrc', 0))

def startDownload(kconfig):
    global global_error
    import kgetcore
    # read settings
    option = YoutubeDlOption(kconfig)
    option.parse()
    # Verify video URL format and convert to "standard" format
    video_url_cmdl = kgetcore.getSourceUrl()
    video_url_mo = const_video_url_re.match(video_url_cmdl)
    if video_url_mo is None:
        video_url_mo = const_video_url_re.match(urllib.unquote(video_url_cmdl))
    if video_url_mo is None:
        exit_with_error('URL does not seem to be a youtube video URL. If it is, report a bug.')
        return
    video_url_id = video_url_mo.group(2)
    video_url = const_video_url_str % video_url_id

    video_format = None

    if option.best_quality:
        video_format = const_best_quality_format
        video_extension = '.mp4'
    else:
        video_extension = '.flv'

    if video_format is not None:
        video_url = '%s%s' % (video_url, const_video_url_format_suffix % video_format)

    # Get account information if any
    account_username = None
    account_password = None
    if option.enable_login:
        if option.use_netrc:
            try:
                info = netrc.netrc().authenticators('youtube')
                if info is None:
                    exit_with_error('no authenticators for machine youtube.')
                    return
                account_username = info[0]
                account_password = info[2]
            except IOError:
                exit_with_error('unable to read .netrc file.')
                return
            except netrc.NetrcParseError:
                exit_with_error('unable to parse .netrc file.')
                return
        else:
            # TODO: check
            account_username = option.username
            account_password = option.password

    # Install cookie and proxy handlers
    urllib2.install_opener(urllib2.build_opener(urllib2.ProxyHandler()))
    urllib2.install_opener(urllib2.build_opener(urllib2.HTTPCookieProcessor()))

    # Log in and confirm age if needed
    if account_username is not None:
        kgetcore.setTextStatus('Logging in...')
        url = const_login_url_str % video_url_id
        post = const_login_post_str % (video_url_id, account_username, account_password)
        reply_page = download_step(True, 'Logging in', 'unable to log in', url, post)
        if global_error:
            return
        if const_bad_login_re.search(reply_page) is not None:
            exit_with_error('unable to log in')
            return

        url = const_age_url_str % video_url_id
        post = const_age_post_str % video_url_id
        download_step(False, 'Confirming age', 'unable to confirm age', url, post)
        if global_error:
            return

    # Install cookie and proxy handlers
    urllib2.install_opener(urllib2.build_opener(urllib2.ProxyHandler()))
    urllib2.install_opener(urllib2.build_opener(urllib2.HTTPCookieProcessor()))

    # Retrieve video webpage
    video_webpage = download_step(True, 'Retrieving video webpage', 'Unable to retrieve video webpage', video_url)
    if global_error:
        return

    # Extract video title
    video_title = extract_step('Extracting video title', 'Unable to extract video title', const_video_title_re, video_webpage)
    if global_error:
        return

    # Extract needed video URL parameters
    video_url_t_param = extract_step('Extracting URL "t" parameter', 'Unable to extract URL "t" parameter', const_url_t_param_re, video_webpage)
    if global_error:
        return
    video_url_real = const_video_url_real_str % (video_url_id, video_url_t_param)
    if video_format is not None:
        video_url_real = '%s%s' % (video_url_real, const_video_url_format_suffix % video_format)

    # Rebuild filename
    if option.use_literal:
        prefix = title_string_touch(video_title)
        video_filename = '%s-%s%s' % (prefix, video_url_id, video_extension)
    else:
        prefix = title_string_norm(video_title)
    video_filename = '%s%s' % (prefix, video_extension)
    #video_filename = '%s%s' % (video_url_id, video_extension)

    # Check name
    if not video_filename.lower().endswith(video_extension):
        kgetcore.setTextStatus('Warning: video file name does not end in %s\n' % video_extension)

    # Add to kget
    kgetcore.addTransfer(video_url_real, video_filename)
    kgetcore.finish()

youtube_option = 0

def configureScript(widget, kconfig):
    from kget_youtubedl_option import YoutubeDlOptionWidget
    global youtube_option
    youtube_option = YoutubeDlOptionWidget(kconfig)
    widget.setWidget(youtube_option)

def configurationAccepted(widget, kconfig):
    from kget_youtubedl_option import YoutubeDlOptionWidget
    global youtube_option
    youtube_option.saveSetting()
