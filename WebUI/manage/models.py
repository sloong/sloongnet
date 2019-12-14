from django.db import models

# Create your models here.
class ServerList(models.Model):
    name = models.CharField(max_length=40,null=False)
    notes = models.CharField(max_length=400,null=False)
    uuid = models.UUIDField(primary_key=True)
    ip = models.GenericIPAddressField(null=False)
    port = models.IntegerField()
    tag = models.IntegerField()

class TagList(models.Model):
    name = models.CharField(max_length=40,null=False)
    notes = models.CharField(max_length=40)

