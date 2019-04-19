﻿using System.Collections;
using System.Collections.Generic;
using UnityEngine;

class PlayerCollider : MonoBehaviour
{
    public PlayerView view;
    bool isTouchedTerrain;
    private void Awake()
    {
    }
    private void OnCollisionEnter(Collision collision)
    {
        /*if (!view.isMoving)
            return;*/
        if (collision.gameObject.GetComponent<Terrain>() != null)
        {
            isTouchedTerrain = true;
        }
    }
    private void OnCollisionStay(Collision collision)
    {
        /*if (!view.isMoving)
            return;*/
        //         if (collision.gameObject.GetComponent<Terrain>() != null)
        //         {
        //             transform.position += Vector3.up * 0.01f;
        //         }
    }
    private void OnCollisionExit(Collision collision)
    {
        /*if (!view.isMoving)
            return;*/
        if (collision.gameObject.GetComponent<Terrain>() != null)
        {
            isTouchedTerrain = false;
        }
    }
    private void Update()
    {
        //if (!view.isMoving)
        //    return;
        if (!isTouchedTerrain)
        {
            var dir = Vector3.down * 0.1f;
            RaycastHit hit;
            if (Physics.Raycast(transform.position, dir, out hit, view.Height))
            {
                if (hit.collider.gameObject.GetComponent<Terrain>() != null)
                {
                    transform.position -= dir;
                    return;
                }
            }
            else if (Physics.Raycast(transform.position, -dir, out hit, 500))
            {
                transform.position = hit.point + view.HeightV;
                return;
            }
            transform.position += dir;
        }
    }
}
public class PlayerView : MonoBehaviour
{
    GameObject player = null;
    enum PlayerState
    {
        Stand,
        Run,
        Jump,
        None,
    }
    float moveSpeed = 6.4f;
    float g = 9.80f;
    float tick = 0;
    Animator controller;
    const string stateName = "state";
    PlayerState currentState;
    PlayerState prevState = PlayerState.Stand;
    bool moveForward = false;
    public bool jumpUp = false;
    public bool isMoving = false;
    public GameObject Player
    {
        get; private set;
    }
    public float Height
    {
        get;
        private set;
    }
    public Vector3 HeightV
    {
        get;
        private set;
    }
    // Use this for initialization
    void Start()
    {
        player = GameObject.Instantiate(Resources.Load<GameObject>("PlayerModel/Player"));
        player.transform.SetParent(null, false);
        player.transform.position = new Vector3(100, 300, 100);
        controller = player.GetComponent<Animator>();
        var collider = player.AddComponent<CapsuleCollider>();
        collider.center = Vector3.zero;
        collider.height = 0.1f;
        collider.radius = 1f;
        controller.SetInteger(stateName, 0);
        player.AddComponent<PlayerCollider>().view = this;
        var rig = player.AddComponent<Rigidbody>();
        Height = 0.5f;
        HeightV = new Vector3(0, 0.5f, 0);
        rig.useGravity = false;
        Player = player;
        rig.constraints = RigidbodyConstraints.FreezeAll;
        tick = 0;
        currentState = PlayerState.Stand;
        Camera.main.transform.SetParent(player.transform, false);
        Camera.main.transform.localPosition = new Vector3(0, 4, -4);
    }

    // Update is called once per frame
    void Update()
    {
        if (currentState != PlayerState.Jump)
        {
            if (Input.GetKey(KeyCode.W))
            {
                Move(true);
            }
            else if (Input.GetKey(KeyCode.S))
            {
                Move(false);
            }
            else
            {
                Stand();
            }
            if (Input.GetKey(KeyCode.Space))
            {
                Jump(true);
            }
            else if (Input.GetKey(KeyCode.LeftShift))
            {
                Jump(false);
            }
        }
        if (Input.GetKey(KeyCode.A))
        {
            Rotate(true);
        }
        else if (Input.GetKey(KeyCode.D))
        {
            Rotate(false);
        }
        if (Input.mouseScrollDelta.y >= 0.67f)
        {
            SetView(true);
        }
        else if (Input.mouseScrollDelta.y < -0.67f)
        {
            SetView(false);
        }
        UpdateState();
    }
    private void UpdateState()
    {
        if (currentState == PlayerState.Jump)
        {
            tick += Time.deltaTime;
            Vector3 offset = Vector3.up * moveSpeed * (jumpUp ? 1 : -1) * Time.deltaTime / 5f;
            offset = player.transform.TransformVector(offset);
            player.transform.position += offset;
            if (tick > 1.0f)
            {
                tick = 0f;
                switch (prevState)
                {
                    case PlayerState.Run:
                        Move(moveForward);
                        break;
                    default:
                        Stand();
                        break;
                }
            }
        }
        if (isMoving)
        {
            Vector3 offset = Vector3.forward * moveSpeed * (moveForward ? 1 : -1) * Time.deltaTime;
            offset = player.transform.TransformVector(offset);
            player.transform.position += offset;
        }
    }
    private void Stand()
    {
        prevState = currentState;
        currentState = PlayerState.Stand;
        controller.SetInteger(stateName, 0);
        isMoving = false;
    }
    private void Move(bool forward)
    {
        moveForward = forward;
        prevState = currentState;
        currentState = PlayerState.Run;
        controller.SetInteger(stateName, 1);
        isMoving = true;
    }
    private void Rotate(bool left)
    {
        player.transform.Rotate(Vector3.up, left ? -1f : 1f);
    }
    private void SetView(bool up)
    {
        Camera.main.transform.Rotate(Vector3.right, up ? 2f : -2f);
    }
    private void Jump(bool up)
    {
        jumpUp = up;
        prevState = currentState;
        currentState = PlayerState.Jump;
        controller.SetInteger(stateName, 2);
        tick = 0;
    }
}
