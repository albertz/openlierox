#!/usr/bin/python

import os, sys, gdata.youtube, gdata.youtube.service, gdata.geo, gdata.media


if len(sys.argv) < 4:
	print "Usage: YoutubeUpload.py username password filepath \"video name\""
	sys.exit(0)

yt_service = gdata.youtube.service.YouTubeService()
yt_service.email = sys.argv[1]
yt_service.password = sys.argv[2]
yt_service.source = "OpenLieroX-video-upload-bot"
yt_service.client_id = "OpenLieroX-video-upload-bot"
yt_service.developer_key = "AI39si4bJZlcHQyoZdH5tmUoeEeCw3dt3cieZ-XRRBFy4kf2XFnyZozdhsidRDdggP-DjD9w6mlRX8StKOIzo3SQnbGtsK7d1Q"
yt_service.ProgrammaticLogin()

# prepare a media group object to hold our video's meta-data
my_media_group = gdata.media.Group(
	title=gdata.media.Title(text=sys.argv[4]),
	description=gdata.media.Description(description_type='plain', text=sys.argv[4]),
	keywords=gdata.media.Keywords(text='games'),
	category=gdata.media.Category(
			text='Games',
			scheme='http://gdata.youtube.com/schemas/2007/categories.cat',
			label='Games'),
	player=None
)


# prepare a geo.where object to hold the geographical location
# of where the video was recorded
#where = gdata.geo.Where()
#where.set_location((37.0,-122.0))

# create the gdata.youtube.YouTubeVideoEntry to be uploaded
video_entry = gdata.youtube.YouTubeVideoEntry(media=my_media_group)

new_entry = yt_service.InsertVideoEntry(video_entry, sys.argv[3])



